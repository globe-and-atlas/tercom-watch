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
- [x] Artificial horizon line and pitch ladder render in upper viewport
- [x] Accelerometer service updates artificial horizon pitch and roll angles
- [x] Reticle ring renders in `GColorChromeYellow` / `GColorYellow` enclosing digital time digits
- [x] Time displays hours and minutes in 12h or 24h format matching user watch settings
- [x] Top telemetry bar displays `TERCOM: TRK-04` and altitude readout
- [x] Bottom telemetry bar displays heading `HDG`, step count `STEPS`, and battery level `BATT`
- [x] Wrist flick / tap event triggers 5-second 30 FPS active terrain scroll animation
- [x] Watchface idles in 1-minute tick mode when wrist is still to conserve battery
- [x] Automated script installs watchface on Emery emulator and captures native screenshots
- [x] Store metadata (`description.txt`, `release-notes.txt`, small and large icons) are generated
- [x] `execution/publish_release.py` runs dry-run validation successfully
- [x] Published to RePebble Appstore with active store listing

## Progress Log

### 2026-10-18 08:07 — Project Scaffolding
- Initialized `tercom-watch` via `create_project.py` with `workflow-python` profile.
- Verified `.gitignore` covers `.lock-waf*`, `.tmp/`, `dist/`, `build/`.
- Updated `SESSION.md` and initialized ISC-compliant `task.md`.

### 2026-10-18 10:00 — Implementation, Testing & RePebble Release
- Developed C source code (`watchface/src/c/config.h`, `watchface/src/c/tercom_renderer.h`, `watchface/src/c/tercom_renderer.c`, `watchface/src/c/main.c`).
- Verified zero-warning compilation on Pebble SDK 4.33.1 (ARM Cortex-M4) using only 3.4 KB RAM footprint (127 KB heap free).
- Booted Emery emulator, verified rendering of 3D topographic wireframe mesh, pitch ladder, and yellow reticle.
- Captured native 200×228 screenshot `store/emery_tercom.png`.
- Generated 28×28 and 144×144 vector store icons with `execution/create_store_assets.py`.
- Verified dry-run and successfully executed `python3 execution/publish_release.py`.
- App published live to RePebble Appstore: `https://apps.rePebble.com/c6f081e1607448bf92df7de4`.
