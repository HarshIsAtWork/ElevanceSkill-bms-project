// bms_engine.cpp - Task 1: Modular Battery Management Engine
#include "bms_engine.h"

// ---- Hardware mapping (ADC1 only: ADC2 is unusable while WiFi is on) -------
static const uint8_t CELL_PINS[] = {32, 33, 34, 35};
static const uint8_t NUM_PINS    = sizeof(CELL_PINS) / sizeof(CELL_PINS[0]);
static const uint8_t NUM_ADC_CELLS = (NUM_CELLS < NUM_PINS) ? NUM_CELLS : NUM_PINS;

// Pot travel (0..4095 raw) is mapped onto a Li-ion NMC cell voltage window.
static const uint16_t CELL_MV_MIN = 3000;
static const uint16_t CELL_MV_MAX = 4200;
static const uint16_t ADC_MAX     = 4095;

// ---- Voltage -> SoC lookup (illustrative NMC curve; replace with the
// ---- datasheet curve of the specific cell you name in the report) ----------
struct SocPoint { uint16_t mv; uint8_t soc; };
static const SocPoint SOC_TABLE[] = {
  {3000,   0}, {3400,   5}, {3500,  10}, {3600,  20}, {3700,  35},
  {3800,  50}, {3900,  65}, {4000,  80}, {4100,  90}, {4200, 100}
};
static const uint8_t SOC_TABLE_LEN = sizeof(SOC_TABLE) / sizeof(SOC_TABLE[0]);

// ---- Module state ----------------------------------------------------------
static BmsSnapshot g_snap;
static int32_t  g_imb_ema      = 0;      // smoothed imbalance (mV)
static int32_t  g_imb_ref      = 0;      // EMA value at last trend evaluation
static uint32_t g_last_sample  = 0;
static uint32_t g_last_trend   = 0;
static bool     g_first        = true;

// ---- Helpers ---------------------------------------------------------------
static uint8_t soc_from_mv(uint16_t mv) {
  if (mv <= SOC_TABLE[0].mv) return SOC_TABLE[0].soc;
  if (mv >= SOC_TABLE[SOC_TABLE_LEN - 1].mv) return SOC_TABLE[SOC_TABLE_LEN - 1].soc;
  for (uint8_t i = 1; i < SOC_TABLE_LEN; i++) {
    if (mv <= SOC_TABLE[i].mv) {
      const SocPoint& a = SOC_TABLE[i - 1];
      const SocPoint& b = SOC_TABLE[i];
      return a.soc + (uint32_t)(mv - a.mv) * (b.soc - a.soc) / (b.mv - a.mv);
    }
  }
  return 100;
}

// Imbalance matters more near the SoC extremes (an outlier cell is closest to
// over-charge / over-discharge), so the allowed limit tightens there.
static uint16_t threshold_from_soc(uint8_t soc) {
  if (soc <= 15 || soc >= 90) return 50;    // tight
  if (soc <= 30 || soc >= 80) return 100;   // moderate
  return 150;                               // relaxed (flat mid-curve region)
}

// ---- Public functions ------------------------------------------------------
void bms_init() {
  analogReadResolution(12);
  for (uint8_t i = 0; i < NUM_ADC_CELLS; i++) {
    analogSetPinAttenuation(CELL_PINS[i], ADC_11db);
    pinMode(CELL_PINS[i], INPUT);
  }
  memset(&g_snap, 0, sizeof(g_snap));
  g_first       = true;
  g_last_sample = millis();
  g_last_trend  = millis();
}

void bms_sample_cells() {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < NUM_ADC_CELLS; i++) {
    uint32_t raw = analogRead(CELL_PINS[i]);
    uint16_t mv = CELL_MV_MIN + (raw * (CELL_MV_MAX - CELL_MV_MIN)) / ADC_MAX;
    g_snap.cell_mv[i] = mv;
    sum += mv;
  }
  // Cells beyond the available ADC pins are simulated: mean of the real cells
  // plus a fixed deterministic offset, so scaling tests are repeatable.
  uint16_t avg = sum / NUM_ADC_CELLS;
  for (uint8_t i = NUM_ADC_CELLS; i < NUM_CELLS; i++) {
    int16_t off = (int16_t)((i * 37) % 61) - 30;          // -30..+30 mV
    int32_t mv  = (int32_t)avg + off;
    if (mv < CELL_MV_MIN) mv = CELL_MV_MIN;
    if (mv > CELL_MV_MAX) mv = CELL_MV_MAX;
    g_snap.cell_mv[i] = (uint16_t)mv;
  }
}

void bms_analyze() {
  // Single O(N) pass: min, max, sum
  uint16_t minv = 0xFFFF, maxv = 0;
  uint8_t  mini = 0, maxi = 0;
  uint32_t sum = 0;
  for (uint8_t i = 0; i < NUM_CELLS; i++) {
    uint16_t v = g_snap.cell_mv[i];
    sum += v;
    if (v < minv) { minv = v; mini = i; }
    if (v > maxv) { maxv = v; maxi = i; }
  }
  g_snap.weakest_mv    = minv;   g_snap.weakest_idx   = mini;
  g_snap.strongest_mv  = maxv;   g_snap.strongest_idx = maxi;
  g_snap.imbalance_mv  = maxv - minv;
  g_snap.avg_mv        = sum / NUM_CELLS;
  g_snap.soc_pct       = soc_from_mv(g_snap.avg_mv);
  g_snap.threshold_mv  = threshold_from_soc(g_snap.soc_pct);
  g_snap.breach        = g_snap.imbalance_mv > g_snap.threshold_mv;
  g_snap.timestamp_ms  = millis();

  // Smooth imbalance (EMA, alpha = 1/4), then judge trend on a slower cadence
  if (g_first) {
    g_imb_ema = g_imb_ref = g_snap.imbalance_mv;
    g_snap.trend = TREND_STEADY;
    g_first = false;
  } else {
    g_imb_ema += ((int32_t)g_snap.imbalance_mv - g_imb_ema) / 4;
    if (g_snap.timestamp_ms - g_last_trend >= BMS_TREND_INTERVAL_MS) {
      int32_t d = g_imb_ema - g_imb_ref;
      if      (d >  BMS_TREND_DEADBAND_MV) g_snap.trend = TREND_INCREASING;
      else if (d < -BMS_TREND_DEADBAND_MV) g_snap.trend = TREND_DECREASING;
      else                                 g_snap.trend = TREND_STEADY;
      g_imb_ref    = g_imb_ema;
      g_last_trend = g_snap.timestamp_ms;
    }
  }
}

bool bms_update() {
  uint32_t now = millis();
  if (now - g_last_sample < BMS_SAMPLE_INTERVAL_MS) return false;
  g_last_sample = now;
  bms_sample_cells();
  bms_analyze();
  return true;
}

const BmsSnapshot& bms_get_snapshot() { return g_snap; }
