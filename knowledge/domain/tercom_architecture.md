---
generated_by: "Antigravity AI (Gemini 3.8 Flash)"
timestamp: "2026-10-18T10:01:00-05:00"
---

# Domain Knowledge: TERCOM Vector Architecture

## Concept Overview
TERCOM (Terrain Contour Matching) is inspired by 1970s cruise missile terrain guidance radar and early Evans & Sutherland DIG visual flight simulation systems. It renders a perspective 3D vector wireframe terrain mesh beneath an artificial horizon and flight-path vector reticle.

## Visual Coordinate Mapping (Pebble Time 2 / Emery, 200×228)
- **Display Resolution:** 200 width × 228 height
- **Color Depth:** 64-color GColor8
- **Horizon Line:** Fixed baseline at `y = 116`, with dynamic pitch offset ($\pm 15$ px) and roll angle from accelerometer
- **Terrain Mesh Viewport:** `y` from 116 to 228
  * $N = 11$ lateral grid columns
  * $M = 9$ perspective depth slices with quadratic distance compression
  * Dual-tone green phosphor: `GColorMalachite` (`#00FF55`) for foreground/crests, `GColorIslamicGreen` (`#00AA00`) for depth ribs
- **Flight Reticle Viewport:** Centered at `(100, 64)`, radius 33 px in `GColorChromeYellow` (`#FFAA00`). Solid black background mask isolates the digital clock digits (`GColorMintGreen`, `FONT_KEY_LECO_28_LIGHT_NUMBERS`) from background horizon rungs.

## Power Profile
- **Motionless / Idle:** 1-minute tick update via `tick_timer_service_subscribe(MINUTE_UNIT)`.
- **Active Wake / Flick:** Accelerometer tap triggers 4.5 seconds of 25 FPS forward terrain flight animation with live tilt, consuming negligible power while providing interactive tactile feedback.
- **RAM Footprint:** 3,420 bytes total out of 128 KB available (127.6 KB free heap).
