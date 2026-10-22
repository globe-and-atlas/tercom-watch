---
generated_by: "OpenAI Codex (GPT-6.1-sol)"
timestamp: "2026-10-18T10:29:52-05:00"
---
# Refine TERCOM terrain detail

Goal: make Crags and Alpine closer to the supplied dense vector-wireframe reference while preserving the clock's legibility on the 200×228 Emery display.

Inputs: user reference image, TERCOM 1.0.2 renderer, native Emery emulator.

Execution scripts: `execution/capture_terrain_detail.py`; existing `execution/create_terrain_preview.py` for listing media.

Outputs: updated Pebble renderer, native comparison captures in `.tmp/terrain-detail/`, refreshed listing captures in `store/` if the new version is published.

Safety: preserve other terrain selections and saved preference; avoid editing the active shared emulator; keep private captures in `.tmp/`.

## Validation Contract

- Crags displays additional contour detail in a native Emery capture.
- Alpine displays additional contour detail in a native Emery capture.
- The time remains readable in Crags.
- The time remains readable in Alpine.
- A wrist flick changes the terrain image during active flight.
- Wrist tilt shifts the terrain during active flight.
- Advancing time alone does not trigger terrain scrolling.
- The terrain returns to its pre-flick resting position when active flight ends.
- TERCOM remains a watchface in its package metadata.
- `pebble build` succeeds for Emery.
- A fresh verifier approves the implementation.
- RePebble serves a 1.0.3 PBW with `watchface: true`.
- The public listing loads the refreshed terrain GIF first.

## 2026-10-18 Interaction decision

The user chose to keep TERCOM as a watchface. Pebble's current TouchService guide restricts raw touch to watchapps, so the supported watchface interaction is wrist flick plus accelerometer tilt with a return to the resting view. Source: https://developer.repebble.com/guides/events-and-services/touch/
