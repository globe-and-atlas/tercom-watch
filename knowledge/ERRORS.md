# Errors

Record deterministic errors, root causes, and fixes here.

## 2026-10-18 — PBW manifest lookup
- Error: ZIP lookup for root-level `manifest.json` raised `KeyError`.
- Cause: Pebble PBWs place the platform manifest at `emery/manifest.json`; app metadata is at `appinfo.json`.
- Fix: read `appinfo.json` for UUID, version, and target platform before using the public PBW.
- Graduated-to: PBW package inspection in the listing media procedure.

## 2026-10-18 — Public PBW version mismatch
- Error: downloaded RePebble 1.0.1 PBW failed the embedded `versionLabel == 1.0.1` check.
- Cause: pending inspection of the public PBW's `appinfo.json`; listing path version alone is insufficient proof of embedded version.
- Next: inspect embedded metadata and binary identity before capturing or changing the listing.
- Classification: deterministic package discrepancy.

## 2026-10-18 — Listing script replacement patch
- Error: one `apply_patch` request rejected a delete and add targeting the same path.
- Cause: patch tool disallows multiple operations on one path in one patch.
- Fix: replace the file in separate edit operations.
- Graduated-to: editing workflow only; the rejected patch changed no files.

## 2026-10-18 — Clean TERCOM build version discrepancy
- Error: clean `git archive HEAD` build succeeded, but PBW validation reported an embedded version other than 1.0.1.
- Cause: another project commit advanced `HEAD` to 1.0.2 between inspection and archive creation.
- Fix: pin the 1.0.1 source commit `b57b27a` for this listing media build; leave 1.0.2 work untouched.
- Classification: deterministic build discrepancy.

## 2026-10-18 — Incremental PBW retains old version label
- Error: `pebble build` succeeded for package version 1.0.2, but `build/watchface.pbw` still embedded `versionLabel: 1.0.1`.
- Cause: Pebble incremental build did not refresh generated appinfo after `package.json` version changed.
- Fix: run `pebble clean` before `pebble build` or publish; verify the PBW's `appinfo.json` matches the intended version.
- Graduated-to: release preflight in `execution/publish_release.py`.

## 2026-10-18 — RePebble rejected same-version release upload
- Error: `pebble publish` returned HTTP 400, "Version 1.0.2 already exists for this app".
- Cause: the CLI creates a new release and cannot replace the existing 1.0.2 release, even with `--replace-screenshots`.
- Fix: use the developer listing editor for screenshot and description changes; a separate version bump would be needed to replace the PBW.
- Graduated-to: listing update procedure.
## 2026-10-18 10:36 — Resting terrain comparison failed

- Error: `python3 execution/capture_terrain_detail.py` raised `ValueError: Terrain did not return to its resting view` after the flick/tilt sequence.
- Status: investigating whether the renderer state differs or the captured terrain crop includes changing HUD pixels.
- Next: inspect exact pixel differences, then correct the state or test boundary and rerun native capture.
- Cause: one QEMU screenshot was captured partway through a redraw; the image lacked most HUD text and did not represent a complete settled frame.
- Fix: the capture script retries the post-flight screenshot until the terrain crop exactly matches the pre-flight image. The rerun passed.
- Graduated-to: native capture procedure in `execution/capture_terrain_detail.py`.

## 2026-10-18 10:41 — Public page still shows 1.0.2 after 1.0.3 publication

- Observation: the Developer Dashboard marks 1.0.3 Published, but the public TERCOM page still reports 1.0.2 after reload.
- Classification: possible storefront cache or propagation delay; no conclusion yet.
- Next: recheck the public page and search card before claiming public availability.
- Resolution: the public detail page later showed 1.0.3 and linked a downloadable PBW whose embedded metadata identifies version 1.0.3 and `watchface: true`.

## 2026-10-18 10:42 — Existing listing description was unchanged by CLI publish

- Observation: the 1.0.3 release was created, but the Developer Dashboard listing editor still showed the 1.0.2 description.
- Cause: the release upload did not apply the `--description` text to the existing listing.
- Fix: saved the updated description directly in the listing editor.
- Graduated-to: `knowledge/procedural/release_tercom.md`.

## 2026-10-27 — Dunes negative index
- Error: deeper rows could index `s_cross` with a negative value.
- Cause: C remainder preserves a negative dividend's sign; adding 16 was insufficient for row 11.
- Fix: normalize remainder with `(value % 8 + 8) % 8`. The pure terrain model is exercised across 5 terrains × 21 columns × 12 rows × 8 phases under address/undefined sanitizers.
- Graduated-to: host model tests and `terrain_model.c`.

## 2026-10-27 — Clock mask clipped scales
- Error: enlarged clock window cut off digits in nearby scale labels.
- Fix: move labels outside the window and reject complete text rectangles that intersect it during tilt. Native resting and tilted captures verified the clearance.

## 2026-10-27 — Release upload requires separate gallery verification
- Observation: CLI reported five uploaded screenshots but the public gallery still served the previous images. Replaced gallery through the dashboard; public propagation must be checked separately.
- Prevention: compare decoded public media against the intended release files before claiming screenshot publication.
- The publish command rebuilds internally, so scrub TERCOM_TEST_* for that subprocess as well as the initial production build.

## 2026-11-09 — Private Diorite emulator startup and clock baseline

Opening the protocol socket immediately after launching private Diorite QEMU led to a watch-version timeout. Wait on the separate debug serial stream for the firmware startup marker before the first Pebble command. Set the emulator clock before launching TERCOM. Setting it after launch can leave the first resting frame on a stale watch minute until an interaction refreshes telemetry. Both changes produced a stable native rest/flight/rest replay.
