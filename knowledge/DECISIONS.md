# Decisions

## Initial Decision

- Template profile: `workflow-python`
- Deploy target: `local-only`
- Runtime: `python`

## 2026-10-18 — Keep TERCOM a watchface

- Decision: keep `watchface: true` and use wrist flick plus accelerometer tilt for a temporary terrain view that returns to rest.
- Alternative considered: convert TERCOM to a watchapp for finger drag via `TouchService`.
- Reason: the user explicitly chose to preserve the watchface after the current Pebble touch restriction was explained.
