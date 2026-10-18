"""Fetch and validate the public TERCOM PBW used for native listing captures."""

from __future__ import annotations

import hashlib
import json
import urllib.request
from pathlib import Path
from zipfile import ZipFile


ROOT = Path(__file__).resolve().parents[1]
TMP = ROOT / ".tmp" / "listing"
PUBLIC_PBW = TMP / "tercom-public-1.0.1.pbw"
PUBLIC_URL = (
    "https://appstore-api.repebble.com/api/assets/pbw/"
    "c6f081e1607448bf92df7de4/1.0.1/"
    "12feaa91-4356-461a-bc70-4d6c99661b10.pbw"
)
EXPECTED_UUID = "a05c6357-104a-4090-bba4-0a1cb7606927"


def main() -> None:
    TMP.mkdir(parents=True, exist_ok=True)
    with urllib.request.urlopen(PUBLIC_URL, timeout=30) as response:
        payload = response.read(2_000_001)
    if len(payload) > 2_000_000:
        raise ValueError("Public PBW exceeded 2 MB bound")
    PUBLIC_PBW.write_bytes(payload)
    with ZipFile(PUBLIC_PBW) as archive:
        info = json.loads(archive.read("appinfo.json"))
        assert "emery/pebble-app.bin" in archive.namelist()
    if info["uuid"] != EXPECTED_UUID:
        raise ValueError("Public PBW UUID differs from TERCOM listing")
    if info["versionLabel"] != "1.0.1":
        raise ValueError("Public PBW version is not 1.0.1")
    if info["targetPlatforms"] != ["emery"]:
        raise ValueError("Public PBW has an unexpected target platform")
    print(f"Public PBW: {PUBLIC_PBW}")
    print(f"SHA-256: {hashlib.sha256(payload).hexdigest()}")


if __name__ == "__main__":
    main()
