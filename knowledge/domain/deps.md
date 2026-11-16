---
generated_by: "OpenAI Codex (gpt-6.1-sol)"
timestamp: "2026-10-27T18:12:03-05:00"
---

# TERCOM build dependencies

- Pebble SDK 4.33.1 / installed Pebble CLI builds the Emery C app and bundles PebbleKit JS.
- `@rebble/clay` 1.1.0 is pinned in `watchface/package.json` and package-lock.json. Added 2026-10-27 to render phone settings for palettes, motion, track, and telemetry. Install with `npm install --prefix watchface`.
- Python Pillow is used by the existing native media scripts and the screenshot comparison. It is a build/QA dependency, not watch runtime code.

Persisted watch settings are authoritative at startup. The phone sends a complete normalized settings dictionary only after a successful settings-page save. This avoids replacing a wrist-selected terrain with the phone's last saved terrain on every watchface launch.
