// sketch.ino - Task 1 demo / test harness
#include "bms_engine.h"

#define RUN_BENCHMARK 0   // set to 1, run once per NUM_CELLS value, record results

static const char* trendName(TrendState t) {
  switch (t) {
    case TREND_INCREASING: return "INCREASING";
    case TREND_DECREASING: return "DECREASING";
    default:               return "STEADY";
  }
}

static void printSnapshot(const BmsSnapshot& s) {
  Serial.print("Cells(mV):");
  for (uint8_t i = 0; i < NUM_CELLS; i++) { Serial.print(' '); Serial.print(s.cell_mv[i]); }
  Serial.println();
  Serial.printf("Weakest: C%u %umV | Strongest: C%u %umV | Imbalance: %umV | Limit: %umV | %s\n",
                s.weakest_idx + 1, s.weakest_mv, s.strongest_idx + 1, s.strongest_mv,
                s.imbalance_mv, s.threshold_mv, s.breach ? "BREACH" : "OK");
  Serial.printf("Avg: %umV | SoC: %u%% | Trend: %s\n\n", s.avg_mv, s.soc_pct, trendName(s.trend));
}

#if RUN_BENCHMARK
static void runBenchmark() {
  const uint32_t N = 1000;
  Serial.printf("=== Benchmark: NUM_CELLS=%d ===\n", NUM_CELLS);
  Serial.printf("sizeof(BmsSnapshot)=%u bytes\n", (unsigned)sizeof(BmsSnapshot));
  Serial.printf("Free heap: %u bytes\n", (unsigned)ESP.getFreeHeap());

  uint32_t t0 = micros();
  for (uint32_t i = 0; i < N; i++) bms_sample_cells();
  uint32_t t1 = micros();
  for (uint32_t i = 0; i < N; i++) bms_analyze();
  uint32_t t2 = micros();

  Serial.printf("sample_cells: %.2f us/call\n", (t1 - t0) / (float)N);
  Serial.printf("analyze:      %.2f us/call\n", (t2 - t1) / (float)N);
}
#endif

void setup() {
  Serial.begin(115200);
  bms_init();
#if RUN_BENCHMARK
  runBenchmark();
#endif
}

void loop() {
  static uint32_t lastPrint = 0;
  if (bms_update() && millis() - lastPrint >= 500) {
    lastPrint = millis();
    printSnapshot(bms_get_snapshot());
  }
}
