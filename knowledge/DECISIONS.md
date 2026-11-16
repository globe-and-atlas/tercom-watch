# Decisions

## Initial Decision

- Template profile: `workflow-python`
- Deploy target: `local-only`
- Runtime: `python`

## 2026-10-18 — Keep TERCOM a watchface

- Decision: keep `watchface: true` and use wrist flick plus accelerometer tilt for a temporary terrain view that returns to rest.
- Alternative considered: convert TERCOM to a watchapp for finger drag via `TouchService`.
- Reason: the user explicitly chose to preserve the watchface after the current Pebble touch restriction was explained.

## 2026-10-27 — Local 1.1.0 instrument update
- Preserve green default, existing UUID, five terrain identifiers, and terrain persistence key 1. New settings use keys 2–9.
- Add Clay settings, label fictional altitude SIM, and offer useful replacement fields and optional date. Heading remains unavailable rather than confidently zero when the compass has no data.
- Isolate deterministic terrain and flight-envelope helpers from Pebble headers for sanitizer tests.
- Retain exact resting-phase restoration; smooth flight speed and filtered tilt while active. No idle animation timer is introduced.
- Keep this iteration local for visual/device review; store upload was not requested for TERCOM in this turn.
