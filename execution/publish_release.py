"""Publish TERCOM to the authenticated RePebble Appstore."""
import argparse
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true", default=False, help="Print publish command without running")
    args = parser.parse_args()

    package = json.loads((ROOT / "watchface/package.json").read_text())
    store = ROOT / "store"

    screenshots = [
        str(store / "emery_tercom.png"),
    ]

    command = [
        "pebble", "publish", "--non-interactive", "--is-published", "--no-gif-all-platforms",
        "--name", "TERCOM",
        "--version", package["version"],
        "--description", (store / "description.txt").read_text().strip(),
        "--release-notes", (store / "release-notes.txt").read_text().strip(),
        "--category", "faces",
        "--icon-small", str(store / "icon-small.png"),
        "--icon-large", str(store / "icon-large.png"),
        "--replace-screenshots",
        "--screenshots", *screenshots,
    ]

    print(f"TERCOM {package['version']} ready to publish.")
    if args.dry_run:
        print("\n[DRY RUN] Would execute the following pebble publish command:")
        print(" ".join(command))
        return

    print("\nExecuting pebble publish...")
    subprocess.run(command, cwd=ROOT / "watchface", check=True)
    print("✓ Successfully published to RePebble!")


if __name__ == "__main__":
    main()
