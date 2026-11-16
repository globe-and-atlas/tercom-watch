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

## 2026-10-18 Touch and resting-view constraint

- The Pebble Time 2 has touch hardware, but Pebble currently restricts `TouchService` to watchapps. TERCOM remains a watchface, so it cannot receive finger drags. Source: https://developer.repebble.com/guides/events-and-services/touch/
- Active flight stores the resting `scroll_phase` before animation. When the timer ends, it restores that phase and clears roll and pitch. Repeated wrist taps extend the same temporary flight view without overwriting its original resting position.
- Minute ticks update clock telemetry and redraw; they do not alter `scroll_phase`.

## 2026-10-27 — Local 1.1.0 instrument update

- `terrain_model.c` is independent of Pebble headers. Sanitizer-backed tests cover five terrains, 21 columns, 12 rows, and eight phases. Dunes modulo is normalized before indexing.
- New native settings occupy persistence keys 2–9; terrain remains key 1. Clay normalizes each field and sends only on Save, preserving wrist-selected terrain at startup.
- Palettes are Green, Amber, Ice blue, Monochrome. Bold time lives in a quiet horizontal mask. Scale labels move outward and are omitted whole when their rectangles would cross the mask during tilt.
- Flight speed follows a symmetric smooth envelope; tilt is filtered. Off creates no animation timer. Short uses 50 frames; Standard uses 112 frames at 25 FPS (approximately 2 and 4.5 seconds). Every finished flight restores its initial scroll phase and zeroes pitch/roll.
- Simulated altitude is labeled SIM. Optional date replaces lower-center steps. The optional course line is illustrative; no terrain or navigation is georeferenced.
