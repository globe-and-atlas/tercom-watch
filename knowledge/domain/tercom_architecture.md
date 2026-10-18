---
generated_by: "Antigravity AI (Gemini 3.8 Flash)"
timestamp: "2026-10-18T10:19:00-05:00"
---

# Domain Knowledge: TERCOM Vector Architecture

## Concept Overview
TERCOM (Terrain Contour Matching) is inspired by 1970s cruise missile terrain guidance radar and early Evans & Sutherland DIG visual flight simulation systems. It renders a perspective 3D vector wireframe terrain mesh beneath an artificial horizon, a 6-line time gauge, and a flight-path vector reticle.

## 5-Terrain Bundle Architecture
The terrain generator features 5 procedural models selectable on-watch via rapid double wrist flick:
1. **`TRK-01:CRAGS` (The Original):** Dual craggy mountain peaks (columns 5 and 16) with a central pass and rolling foothills.
2. **`TRK-02:CANYON`:** High sheer gorge plateaus on the flanks with a deep, flat canyon riverbed floor corridor.
3. **`TRK-03:DUNES`:** Sinuous transverse sand dunes with shifting harmonic ripples.
4. **`TRK-04:CALDERA`:** Circular sunken volcanic crater basin with elevated rim and central cone.
5. **`TRK-05:ALPINE`:** Sawtooth multi-summit alpine massif with jagged cols.

## Time & Gauge Integration
- **6-Line Gauge (Upper Viewport):**
  * Left side: Continuous hour reference scale centered on current hour (`12, 11, 10, 09, 08, 07`).
  * Right side: 5-minute intervals (`:25..:00` for min < 30; `:55..:30` for min >= 30).
  * Top HUD director chevron and horizontal baseline ticks.
- **Flight Reticle:** Centered at `(100, 62)`, radius 33 px in `GColorChromeYellow` (`#FFAA00`). Solid black background mask isolates digital clock digits (`GColorMintGreen`, `FONT_KEY_LECO_28_LIGHT_NUMBERS`) from background rungs.

## Gesture & Storage Mechanics
- Single wrist flick: Wakes up 4.5s 25 FPS active forward flight animation with live horizon banking.
- Quick double wrist flick (within 1 second): Advances `terrain_type`, triggers haptic buzz (`vibes_short_pulse`), and writes selection to flash (`persist_write_int(STORAGE_KEY_TERRAIN, s_telemetry.terrain_type)`).
- RAM Footprint: 4,000 bytes out of 128 KB available (127 KB heap free).
