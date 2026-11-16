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

### 2026-10-18 — Reference-detail terrain refinement
- [x] Add denser contours to Crags.
- [x] Add denser contours to Alpine.
- [x] Keep the time readable in Crags.
- [x] Keep the time readable in Alpine.
- [x] Verify active terrain motion after a wrist flick.
- [x] Verify terrain parallax during wrist tilt.
- [x] Verify time changes do not scroll the terrain.
- [x] Verify terrain returns to its resting position after active flight.
- [x] Verify package metadata still identifies TERCOM as a watchface.
- [x] Pass the Emery build.
- [x] Obtain a fresh verifier review.
- [x] Publish the 1.0.3 PBW as a watchface.
- [x] Verify the public search card loads the refreshed GIF.
- [x] Verify the public detail page offers the 1.0.3 PBW.

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

## TERCOM 1.1.0 — 2026-10-27

Output: release/TERCOM-1.1.0.pbw; native previews in store/preview-1.1.0/. Target: Emery watchface with existing UUID.

- [x] Green remains the default palette.
- [x] Amber, Ice blue, and Monochrome are selectable.
- [x] Resting clock is visually distinct from dimmer scales.
- [x] Five terrain silhouettes remain selectable.
- [x] Dunes calculations avoid negative array indices.
- [x] Flight Off does not schedule animation.
- [x] Short flight lasts two seconds.
- [x] Standard flight lasts 4.5 seconds.
- [x] Completed flight restores the starting terrain phase.
- [x] Tilt sensitivity is selectable.
- [x] Double-tap track switching can be disabled.
- [x] The simulated altitude field is labeled SIM.
- [x] Optional date offers Off, ISO, US, and European formats.
- [x] Selected settings survive restart.
- [x] Native screenshots show four palettes.
- [x] Host tests pass.
- [x] Production PBW contains no fixed-time fixture.
- [x] Independent verifier approves the final package.

Safety: preserve current UUID and terrain storage key; no store publication in this local design iteration.

2026-10-27: local 1.1.0 implementation and package gates passed. Sanitized model tests and phone-settings normalization pass. Private native replay verifies motion Off, durations, tilt movement, exact resting restoration, disabled track switching, settings persistence, and live production clock. Physical-watch feel and phone UI flow remain open.

## Store release 1.1.0 — 2026-10-27

- [x] Release PBW preserves the existing UUID.
- [x] Release PBW identifies version 1.1.0.
- [x] Gallery contains five native 200×228 media files.
- [x] Store copy names four palettes.
- [x] Store copy identifies simulated telemetry.
- [x] Independent release review passes.
- [x] Dashboard marks 1.1.0 Published.
- [x] Public download serves 1.1.0.
- [x] Public gallery matches the new media.

User authorized store publication and new screenshots. Use execution/publish_release.py, store/listing-1.1.0/, and store/description.txt. Preserve production fixtures-off build policy.

## Round 2 and Pebble 2 visual probe — 2026-11-09

- [x] The isolated probe PBW contains a Gabbro binary.
- [x] The isolated probe PBW contains a Diorite binary.
- [x] The Gabbro emulator produces a 260×260 resting frame.
- [x] The Gabbro emulator produces an in-flight frame.
- [x] The Diorite emulator produces a 144×168 resting frame.
- [x] The Diorite emulator produces an in-flight frame.
- [x] An independent reviewer inspects the probe artifacts.

Output: /private/tmp/tercom-2026-11-09/.tmp/platform-probe/. Release approval depends on the user's visual decision.

## TERCOM 1.2.0 store release

- [x] The production PBW identifies version 1.2.0.
- [x] The production PBW targets Emery, Gabbro, and Diorite.
- [x] Emulator flight restores the resting frame on Emery, Gabbro, and Diorite.
- [x] The public description opens with the approved tagline.
- [x] The Time 2 gallery opens with a native mid-flight GIF.
- [x] The Round 2 gallery opens with a native mid-flight GIF.
- [x] The Pebble 2 gallery opens with a native mid-flight GIF.
- [x] The public download matches the reviewed production PBW.
- [x] An independent reviewer checks release evidence.

Output: release/TERCOM-1.2.0.pbw and store/listing-1.2.0/.
