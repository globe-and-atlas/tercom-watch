# TERCOM 1.2.0 — 2026-11-09

**Goal:** Publish the approved Time 2, Round 2, and Pebble 2 release with the flight tagline and mid-flight gallery covers.
**Agent:** OpenAI Codex
**Handoff-from:** OpenAI Codex
**Handoff-type:** continuation
**Status:** Live. The public description, PBW, and three galleries were verified.

The clean 1.2.0 production bundle contains Emery, Gabbro, and Diorite. Its SHA-256 is 3c2076e3d6774d9ad9b308886defb2a6cc60dc7a2182103529d47a37a4149d8c. Both host tests passed. Each emulator recorded distinct flight frames followed by exact resting-frame restoration. Time 2 retains its existing telemetry geometry. Round 2 fits readouts inside the circle. Pebble 2 uses a reduced monochrome mesh.

The existing listing begins with “Tap your wrist and fly over the terrain.” The selected gallery for each requested device leads with native flight footage, followed by four updated stills. An independent reviewer matched the public PBW and all 15 assets to the reviewed files. The store also reports 2 Duo through platform compatibility. The native build and capture scope is Emery, Gabbro, and Diorite. Evidence is under release/1.2.0/ and store/listing-1.2.0/.

S.D.G.: Gives Round 2 and Pebble 2 owners a readable cockpit watchface and lets shoppers see the flight immediately.
Feynman gap: The native emulators verify flight and restoration. Wrist-tap feel and battery cost still need physical-watch evidence.

# TERCOM platform probe — 2026-11-09

**Goal:** Show whether TERCOM reads on Pebble Round 2 and Pebble 2 at native size.
**Agent:** OpenAI Codex
**Handoff-from:** OpenAI Codex
**Handoff-type:** continuation
**Status:** Probe approved and superseded by the published 1.2.0 release.

The isolated target-only build compiled for Gabbro and Diorite. The Round 2 baseline exposed a clipped, off-center Emery layout. The fitted probe centers and scales the round HUD, places telemetry inside the circular crop, and simplifies the monochrome mesh and compact labels. It leaves the working source and production settings intact. The two host checks passed. Private native captures confirmed visible flight motion and pixel-exact restoration on both targets. Independent review verified the exact PBW and native frames.

Artifacts are under /private/tmp/tercom-2026-11-09/.tmp/platform-probe. The fitted PBW SHA-256 is ceed281ce971d4cb1ccfcf4ebfc5999b3f6de666f8536563b3c5a309809c2db7. See TERCOM-native-probe.png, review-shots/verification.json, and layout-probe.patch. Diorite is the original Pebble 2. Flint is the separate Pebble 2 Duo target.

After visual approval, use the native assets to guide the production port. Proposed tagline is “Tap your wrist and fly over the terrain.” The first GIF frame should show flight. Verify Emery alongside the added platforms before any 1.2.0 release.

S.D.G.: Gives the maintainer a wrist-scale comparison to decide how widely to ship TERCOM.
Feynman gap: Native emulators verify motion and layout. Physical tap feel, phone settings, and battery remain open.

# Current update — 2026-10-27

**Goal:** TERCOM 1.1.0 readability, terrain, palettes, motion settings, and telemetry choices.
**Agent:** OpenAI Codex
**Handoff-from:** OpenAI Codex
**Handoff-type:** continuation
**Status:** TERCOM 1.1.0 published; public PBW and refreshed gallery verified.

## Result

- Four palettes, bolder clock, complete dimmed scales, winding Canyon, clearer Caldera rim, layered Alpine, and safe Dunes indexing.
- Clay phone settings select track, motion duration, tilt response, double-tap switching, top telemetry, lower date/steps, and optional illustrative course.
- Two substantive host tests pass (sanitized C model sweep and JS normalization). The private native replay passes motion Off, Short/Standard exact restoration, tilt movement, disabled track switching, and exact saved presentation after restart.
- Native palette/terrain previews use a fixed 23:59 readability fixture in `store/preview-1.1.0/`. `execution/build.py` then cleaned the fixture and produced `release/TERCOM-1.1.0.pbw`. A separate production replay advanced watch time from 20:16 to 20:17 and observed changed clock pixels with identical terrain.
- Independent review found clipped ladder labels beside the enlarged clock; corrected their placement and bounding checks, recaptured, and review passed.
- Store release: public version 1.1.0 and downloaded PBW UUID verified; dashboard Published. Description saved through the listing editor. CLI reported screenshot upload but public images remained old; replaced through dashboard and verified the public animated tour plus four palette screenshots. GIF frames match exactly; PNGs visually match, with 0–106 palette-changed pixels per 45,600 pixels after server processing.
- Evidence: `.tmp/store-live-1.1.0.png`; release media `store/listing-1.1.0/`.
- Next: physical wrist feel, phone settings UI, and battery impact remain unverified.

# Session Log

## Last Known State

**Goal:** Keep TERCOM a Pebble Time 2 watchface with detailed terrain and a temporary wrist-controlled flight view.
**Agent:** OpenAI Codex (GPT-6.1-sol)
**Handoff-from:** Antigravity AI
**Handoff-type:** pickup
**Status:** TERCOM 1.0.3 is published as a watchface. The public 1.0.3 PBW, GIF-first gallery, animated search card, and updated description were verified.
**Tests:** Clean Emery build; private native QEMU motion/restoration/time captures; `python3 -m pytest -q` (1 passed); fresh independent verifier approved.
**Next:** Test on physical Pebble Time 2 when available. Finger drag remains unavailable in watchfaces under current Pebble TouchService policy.
**Closing audit:** The touch request was a product tradeoff: the user chose a persistent watchface, so wrist motion with exact snap-back was implemented and published. Physical wrist feel remains unverified.

## Checkpoint Log

- 2026-10-21 22:41 — commit: feat: update TERCOM to version 1.0.3 with refined terrain wireframes, GIF listing media, and updated metadata | .tmp/.gitkeep,directives/refine_terrain.md,directives/update_listing_media.md,execution/capture_terrain_detail.py,execution/create_terrain_preview.py


S.D.G.: Helps TERCOM wearers read time more easily and tailor a brief terrain-flight experience without losing their resting scene.
Feynman gap: Emulator and host checks do not establish physical wrist feel, paired-phone configuration flow, or battery impact.
