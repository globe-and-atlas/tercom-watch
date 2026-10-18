# Task: tercom-watch

## Objective

Build, test, and publish **TERCOM** — a 1970s phosphor vector CRT topographic guidance watchface for the Pebble Time 2 (`emery`, 200×228 64-color display) — to the RePebble appstore.

## Context

- Target Platform: `emery` (Pebble Time 2, 200×228 64-color memory LCD, Cortex-M4)
- Runtime: C / Pebble SDK v4.33.1 + Python automation
- Visual Aesthetic: Evans & Sutherland military flight simulator & terrain-contour matching vector HUD
- Knowledge Level: heavy
- RePebble App URL: https://apps.rePebble.com/c6f081e1607448bf92df7de4

## Acceptance Criteria

- [x] `watchface/package.json` specifies `"targetPlatforms": ["emery"]`
- [x] `watchface/package.json` specifies `"watchface": true` inside `"watchapp"`
- [x] `watchface/src/c/tercom_renderer.c` and renderer modules compile with `pebble build` for emery with zero errors
- [x] 3D wireframe terrain mesh renders at least 8 perspective contour ridges in `GColorMalachite` and `GColorIslamicGreen`
- [x] 5 selectable terrain models bundled: CRAGS (the original favorite!), CANYON, DUNES, CALDERA, and ALPINE
- [x] Double-tap wrist gesture cycles active terrain track with haptic feedback and saves permanently to watch flash
- [x] Artificial horizon line and 6-line altitude gauge render in upper viewport
- [x] 6-line altitude gauge displays hours reference on left and 5-minute increments on right
- [x] Accelerometer service updates artificial horizon pitch and roll angles
- [x] Reticle ring renders in `GColorChromeYellow` / `GColorYellow` enclosing digital time digits
- [x] Time displays hours and minutes in 12h or 24h format matching user watch settings
- [x] Top telemetry bar displays active track (e.g. `TRK-01:CRAGS`) and altitude readout
- [x] Bottom telemetry bar displays heading `HDG`, step count `STEPS`, and battery level `BATT`
- [x] Wrist flick / tap event triggers 5-second 30 FPS active terrain scroll animation
- [x] Watchface idles in 1-minute tick mode when wrist is still to conserve battery
- [x] Automated script installs watchface on Emery emulator and captures native screenshots for all 5 terrains
- [x] Store metadata and 5-screenshot gallery published to RePebble Appstore

## Progress Log

### 2026-10-18 08:07 — Project Scaffolding
- Initialized `tercom-watch` via `create_project.py` with `workflow-python` profile.
- Verified `.gitignore` covers `.lock-waf*`, `.tmp/`, `dist/`, `build/`.
- Updated `SESSION.md` and initialized ISC-compliant `task.md`.

### 2026-10-18 10:00 — v1.0.0 Release
- Developed C source code (`config.h`, `tercom_renderer.h`, `tercom_renderer.c`, `main.c`).
- Verified zero-warning compilation on Pebble SDK 4.33.1.
- Booted Emery emulator, verified rendering and published to RePebble.

### 2026-10-18 10:10 — v1.0.1 Telemetry & Mountain Horizon Alignment
- Redesigned 6-line gauge to reference time (hours on left, 5-minute increments on right).
- Modeled edge-to-edge terrain with peaks rising cleanly above the horizon line.

### 2026-10-18 10:19 — v1.0.2 The 5-Terrain Bundle Release
- Added 5 distinct terrain models: TRK-01 CRAGS (the original), TRK-02 CANYON, TRK-03 DUNES, TRK-04 CALDERA, and TRK-05 ALPINE.
- Implemented double-tap wrist gesture with short vibration pulse and flash persistence (`STORAGE_KEY_TERRAIN`).
- Captured emulator screenshots for all 5 tracks.
- Published v1.0.2 to RePebble with updated 5-screenshot carousel.
