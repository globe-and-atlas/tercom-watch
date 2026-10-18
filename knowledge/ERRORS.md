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
