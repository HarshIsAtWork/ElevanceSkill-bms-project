# Real-Time Battery Management System

ElevanceSkills embedded internship project. A modular ESP32-based BMS
built incrementally across 6 tasks and integrated into a single Wokwi
project.

## Live Wokwi project
<!-- paste the public Wokwi project link here once created -->

## Tasks

| # | Task | Status |
|---|------|--------|
| 1 | Modular Battery Management Engine | In progress |
| 2 | ... | Not started |
| 3 | ... | Not started |
| 4 | ... | Not started |
| 5 | ... | Not started |
| 6 | ... | Not started |

## Repository structure

```
.
├── firmware/            # sketch.ino + all .h/.cpp modules
│   └── secrets.example.h  # copy to secrets.h, fill in, never commit secrets.h
├── wokwi/                # diagram.json, wokwi.toml
├── docs/
│   └── screenshots/      # Wokwi + Serial Monitor screenshots per task
├── report/
│   └── REPORT.md         # full technical report
├── BUG_LOG.md
├── ASSUMPTIONS.md
├── SELF_ATTESTATION.md
└── .gitignore
```

## Task 1 summary
<!-- one paragraph once Task 1 is confirmed working, plus a link to the
     relevant report section -->

## Build / run

**Wokwi (browser):**
1. Open the project link above, or create a new ESP32 project and paste
   in firmware/sketch.ino, firmware/bms_engine.h, firmware/bms_engine.cpp.
2. Wire per docs/screenshots/task1_wiring.png (or the diagram in
   wokwi/diagram.json).
3. Start simulation.

**Real ESP32 (Arduino IDE):**
1. Install the ESP32 board package.
2. Copy firmware/secrets.example.h to firmware/secrets.h and fill in
   your WiFi/Blynk credentials (needed from Task 5 onward).
3. Open firmware/sketch.ino, select your board and port, upload.
