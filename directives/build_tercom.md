---
generated_by: "Antigravity AI (Gemini 3.8 Flash)"
timestamp: "2026-10-18T08:08:00-05:00"
---

# Directive: build_tercom

## Goal
Build, verify, package, and publish the TERCOM vector topographic radar watchface for the Pebble Time 2 (`emery`, 200×228 display) to the RePebble appstore.

## Inputs
- Architecture specification: 1970s vector CRT / Evans & Sutherland DIG terrain simulation HUD
- Platform: Pebble Time 2 (`emery`, 200×228, 64-color memory LCD)
- Pebble SDK: v4.33.1 (installed locally)
- Pebble CLI: `/Users/danielbally/.local/bin/pebble`

## Tools & Execution Scripts
- `watchface/` C source code
- `execution/emulator_check.py`: boots QEMU emery emulator, installs build, captures screenshots
- `execution/create_store_assets.py`: generates store icons and listing previews
- `execution/publish_release.py`: validates package and runs `pebble publish`

## Success Outputs
- `watchface/build/pebble-app.pbw`
- `store/icon-small.png` (28×28)
- `store/icon-large.png` (144×144)
- `store/emery_tercom.png` (200×228 native emulator screenshot)
- `store/description.txt`
- `store/release-notes.txt`
- Verified RePebble store package

## Edge Cases & Gotchas
- Emery emulator does not have compass hardware; mock heading or check status >= CompassStatusCalibrating.
- Ensure `.lock-waf*` is never committed.
- Low-power mode: Watchface must drop to 1-minute tick handler when motionless to preserve 7+ day battery life.
