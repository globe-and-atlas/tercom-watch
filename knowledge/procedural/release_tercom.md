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

## 2026-10-27 Learnings — 1.1.0
- Use `execution/build.py` for a clean production build. Scrub TERCOM_TEST_* on the final publish subprocess because the CLI rebuilds internally.
- Versioned native gallery is `store/listing-1.1.0/`: first GIF tours options, followed by four palette screenshots. Fixed 23:59 is a media fixture only.
- Verify downloaded public PBW metadata separately from gallery propagation. CLI upload success does not establish that the public gallery changed; use the dashboard gallery editor if necessary, then compare public pixels.

## 2026-11-09 — Three-platform release

Build through execution/build.py. Run execution/capture_release.py with the production PBW and a temporary output path to capture the three galleries serially. The capture validates the initial flight frame against the resting frame and checks exact restoration.

Run execution/publish_release.py with the Pebble SDK Python environment. The script uploads the exact reviewed PBW and model-specific previews together. Edit the description separately in the dashboard. Check the refreshed public model selections, then pass the observed public PBW URL and gallery order to the script's verify mode. Store caches can briefly show the prior description or previews on a previously loaded device selection.
