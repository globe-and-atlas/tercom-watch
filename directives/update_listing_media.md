---
generated_by: "OpenAI Codex (GPT-6.1-sol)"
timestamp: "2026-10-18T08:06:00-05:00"
---
# Update TERCOM listing media

Goal: review the public TERCOM 1.0.1 face and add native Emery screenshots plus an animated GIF to its existing RePebble listing.

Inputs: public TERCOM 1.0.1 PBW, Emery emulator, current listing, RePebble developer account.

Execution scripts: `execution/prepare_listing_media.py`; existing `execution/emulator_check.py` for reference only.

Outputs: `store/listing/` screenshots and GIF; public TERCOM listing with GIF first.

Safety: preserve uncommitted source changes; do not publish the local dirty PBW; keep tokens and temporary captures out of Git.

## Validation Contract

- The source PBW used for captures matches public version 1.0.1.
- Each uploaded image is 200×228 pixels.
- The GIF contains distinct native emulator frames.
- The GIF is the first listing screenshot.
- The listing contains at least two static screenshots.
- The public search card loads the GIF.
- The listing remains at version 1.0.1.
