// bms_engine.h - Task 1: Modular Battery Management Engine
#pragma once
#include <Arduino.h>

// ---- Compile-time configuration -------------------------------------------
#ifndef NUM_CELLS
#define NUM_CELLS 4            // change to 8 / 12 / 16 for the scaling test
#endif

#define BMS_SAMPLE_INTERVAL_MS   100    // how often cells are sampled
#define BMS_TREND_INTERVAL_MS    1000   // how often the trend is re-evaluated
#define BMS_TREND_DEADBAND_MV    5      // ignore smoothed changes below this

// ---- Public data types -----------------------------------------------------
enum TrendState : uint8_t {
  TREND_STEADY = 0,
  TREND_INCREASING,
  TREND_DECREASING
};

struct BmsSnapshot {
  uint16_t   cell_mv[NUM_CELLS];   // per-cell voltage, millivolts
  uint16_t   weakest_mv;
  uint16_t   strongest_mv;
  uint16_t   imbalance_mv;         // strongest - weakest
  uint16_t   threshold_mv;         // active adaptive imbalance limit
  uint16_t   avg_mv;               // pack average cell voltage
  uint32_t   timestamp_ms;
  uint8_t    weakest_idx;
  uint8_t    strongest_idx;
  uint8_t    soc_pct;              // estimated from avg cell voltage
  TrendState trend;
  bool       breach;               // imbalance_mv > threshold_mv
};

// ---- Public interface ------------------------------------------------------
void bms_init();                               // call once in setup()
bool bms_update();                             // non-blocking; true = new snapshot
const BmsSnapshot& bms_get_snapshot();         // read-only view for other modules

// Exposed separately so the scaling report can time them independently.
void bms_sample_cells();                       // ADC read (+ simulated cells)
void bms_analyze();                            // min/max, imbalance, SoC, threshold, trend
