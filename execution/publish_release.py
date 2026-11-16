"""Publish the reviewed TERCOM PBW and native galleries through Pebble SDK authentication."""
import argparse
from contextlib import ExitStack
import json
from pathlib import Path
import sys
from zipfile import ZipFile

import requests
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT.parent / "execution"))
from publish_watchface_updates import API, PublishCommand, account_mapping, fetch, save, sha

APP_ID = "c6f081e1607448bf92df7de4"
URL = f"https://apps.repebble.com/tercom_{APP_ID}"
PLATFORMS = {"emery": (200, 228), "gabbro": (260, 260), "diorite": (144, 168)}


def release_inputs():
    package = json.loads((ROOT / "watchface/package.json").read_text())
    version = package["version"]
    uuid = package["pebble"]["uuid"]
    pbw = ROOT / "release" / f"TERCOM-{version}.pbw"
    with ZipFile(pbw) as archive:
        assert archive.testzip() is None
        app = json.loads(archive.read("appinfo.json"))
        assert app["versionLabel"] == version
        assert app["uuid"] == uuid
        assert app["targetPlatforms"] == list(PLATFORMS)
        assert app["watchapp"]["watchface"] is True
    gallery = ROOT / "store" / f"listing-{version}"
    assets = {}
    for platform, size in PLATFORMS.items():
        paths = sorted(gallery.glob(f"{platform}_*.gif"))
        assert len(paths) == 5
        assert paths[0].name == f"{platform}_01_flight.gif"
        for index, path in enumerate(paths):
            with Image.open(path) as image:
                assert image.size == size
                assert image.n_frames >= 5 if index == 0 else image.n_frames == 1
        assets[platform] = paths
    return version, uuid, pbw, assets


def publish(version, uuid, pbw, assets, output):
    guard = output / "upload-inflight.json"
    receipt = output / "store-release.json"
    if guard.exists() or receipt.exists():
        raise RuntimeError("Release already attempted. Reconcile the existing result before any upload.")
    token, mapping = account_mapping()
    assert PublishCommand._lookup_app_id_case_insensitive(mapping, uuid) == APP_ID
    data = {
        "version": version,
        "isPublished": "true",
        "replaceScreenshots": "true",
        "releaseNotes": (ROOT / "store/release-notes.txt").read_text().strip(),
    }
    fingerprint = sha(pbw.read_bytes())
    save(guard, {"app_id": APP_ID, "version": version, "pbw_sha256": fingerprint})
    with ExitStack() as stack:
        files = [("pbwFile", (pbw.name, stack.enter_context(pbw.open("rb")), "application/octet-stream"))]
        for platform, paths in assets.items():
            files.extend((f"screenshots_{platform}", (path.name, stack.enter_context(path.open("rb")), "image/gif"))
                         for path in paths)
        try:
            response = requests.post(f"{API}/api/dashboard/apps/{APP_ID}/releases",
                                     headers={"Authorization": f"Bearer {token}"},
                                     data=data, files=files, timeout=(15, 300))
        except requests.RequestException:
            raise RuntimeError("Upload outcome uncertain. Reconcile before another write.") from None
    if not response.ok:
        raise RuntimeError(f"Release upload returned HTTP {response.status_code}. The upload guard was retained.")
    payload = response.json()
    save(receipt, {"url": URL, "app_id": APP_ID, "version": version, "pbw_sha256": fingerprint,
                   "response": payload, "public_verified": False})
    guard.unlink()
    if payload.get("screenshotResults", {}).get("failed"):
        raise RuntimeError("Release saved. The gallery needs a separate correction.")
    print(f"Release accepted: {URL}")


def verify(version, pbw, assets, output, observed):
    evidence = json.loads(observed.read_text())
    assert evidence["description"].startswith("Tap your wrist and fly over the terrain.")
    download = evidence["download"]
    assert download.startswith(f"{API}/api/assets/pbw/{APP_ID}/{version}/")
    fingerprint = sha(pbw.read_bytes())
    assert sha(fetch(download)) == fingerprint
    for platform, paths in assets.items():
        for path, url in zip(paths, evidence["galleries"][platform], strict=True):
            assert f"/{APP_ID}/{platform}/" in url
            assert sha(fetch(url)) == sha(path.read_bytes()), path.name
    save(output / "public-proof.json", {**evidence, "pbw_sha256": fingerprint})
    receipt = output / "store-release.json"
    save(receipt, {**json.loads(receipt.read_text()), "public_verified": True})
    print(f"Public download, description, and gallery assets verified: {URL}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--verify", type=Path, help="Public browser evidence JSON")
    args = parser.parse_args()
    version, uuid, pbw, assets = release_inputs()
    output = ROOT / "release" / version
    if args.dry_run:
        print(f"TERCOM {version}: {sha(pbw.read_bytes())}; five native previews on {', '.join(assets)}")
    elif args.verify:
        verify(version, pbw, assets, output, args.verify)
    else:
        publish(version, uuid, pbw, assets, output)


if __name__ == "__main__":
    main()
