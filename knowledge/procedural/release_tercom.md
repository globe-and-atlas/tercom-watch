---
generated_by: "OpenAI Codex (GPT-6.1-sol)"
timestamp: "2026-10-18T10:44:00-05:00"
---

# TERCOM release procedure

1. From `watchface/`, run `pebble clean` then `pebble build`. Inspect `appinfo.json` inside `build/watchface.pbw` for matching version, UUID, Emery target, and `watchface: true`.
2. From the project root, run `python3 execution/capture_terrain_detail.py`. It uses a private Emery QEMU and checks active motion, exact resting-view restoration, and time-only stability. Its captures stay in `.tmp/terrain-detail/`.
3. Copy the verified native Crags and Alpine idle captures into `store/emery_tercom.png` and `store/emery_alpine.png`, then run `python3 execution/create_terrain_preview.py` to regenerate the five-track GIF.
4. Run `python3 -m pytest -q` and `git diff --check`.
5. Run `python3 execution/publish_release.py --dry-run`, then `python3 execution/publish_release.py` for a new version. Same-version release uploads are rejected; use the Developer Dashboard for listing-only edits.
6. Reopen the Developer Dashboard and check Latest Release status. On an existing listing, `pebble publish --description` did not replace the description, so edit and save it in the listing editor. Check screenshot order there. Check the public search card's image source for the new first-position GIF. Check the public detail page's version and PBW download separately; it may lag the dashboard and search card.
