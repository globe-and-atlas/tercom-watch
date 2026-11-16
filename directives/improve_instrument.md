---
generated_by: "OpenAI Codex (gpt-6.1-sol)"
timestamp: "2026-10-27T18:10:00-05:00"
---

# Improve TERCOM instrument

Use `task.md` as the 1.1.0 validation contract. Keep the terrain model independent of Pebble headers so host sanitizers can exercise every terrain, grid coordinate, and phase.

1. Run `python3 -m unittest discover -s tests -v` for model bounds, flight envelope, and phone setting normalization.
2. For native 23:59 readability captures only, clean then build from `watchface/` with `TERCOM_TEST_MINUTE=1439`.
3. Run `python3 execution/capture_update.py`. It boots a private Emery flash image, captures the four palettes and five terrains, checks motion Off, restoration after Short and Standard flight, disabled track switching, and presentation persistence after restart.
4. Inspect scale labels beside the clock window. Labels must be complete, including during tilt. Inspect date clearance above the lower telemetry bar.
5. Run `python3 execution/build.py` to clean away fixtures and write the production PBW under `release/`.
6. Run a fresh independent review before accepting the local update. Do not publish until requested.

The native previews have a fixed 23:59 clock for readability testing. They are not evidence of the production clock's live minute updates. Physical wrist feel and battery measurements remain separate device checks.
