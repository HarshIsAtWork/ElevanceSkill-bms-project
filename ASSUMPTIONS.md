# Assumptions & Design Decisions

Document every non-obvious choice and why it was made. This is graded
material, not an afterthought.

## Task 1 — Modular Battery Management Engine

- Cell chemistry: Li-ion NMC, 3.0-4.2 V per cell. <!-- name the exact
  cell/datasheet you're basing this on -->
- Physical inputs: 4 real potentiometers on ESP32 ADC1 pins
  (GPIO 32/33/34/35). ADC1 was chosen because ADC2 is unusable while WiFi
  is active (needed for Tasks 5-6).
- Cells beyond 4: software-simulated as (average of real cells + a
  small deterministic offset) for the scaling test, since Task 1's
  scaling requirement concerns the engine's algorithmic complexity
  (memory/time vs N), not additional physical sensor acquisition.
- Voltage representation: uint16_t millivolts, no floats in the core
  path, to keep memory and timing arguments simple to state.
- SoC estimation: voltage-to-SoC lookup table with linear
  interpolation. <!-- state your real source/justification here -->
- Adaptive threshold: tightens near SoC extremes (<=15% or >=90%),
  loosens in the mid-range. <!-- justify the specific mV values you use -->
- Trend detection: imbalance is smoothed with an EMA and evaluated
  against a deadband every 1s, to avoid ADC-noise-driven flicker between
  states.

## Task 2

<!-- fill in as you go -->
