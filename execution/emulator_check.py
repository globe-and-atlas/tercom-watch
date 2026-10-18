"""Emulator check and screenshot capture for TERCOM (Pebble Time 2 / emery).

Builds with test fixtures (10:48, HDG 284°, STEPS 9,210, BATT 86%, ALT 1,280FT),
boots the Emery QEMU emulator cleanly, captures native 200x228 screenshots to
.tmp/emulator/ and store/, and verifies image output.
"""
import os
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FACE = ROOT / "watchface"
OUT = ROOT / ".tmp" / "emulator"
STORE = ROOT / "store"


def run_cmd(cmd: list[str], cwd: Path = FACE, timeout: int = 120, env: dict | None = None) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=timeout, env=env)


def clean_emulator():
    print("Clearing any stale emulator processes...")
    run_cmd(["pebble", "kill"], timeout=30)
    subprocess.run(["pkill", "-f", "pypkjs"], capture_output=True)
    subprocess.run(["pkill", "-f", "qemu-pebble"], capture_output=True)
    time.sleep(2)


def build_with_fixtures(minute: int = 648, heading: int = 284, steps: int = 9210, battery: int = 86, alt: int = 1280):
    print(f"Building TERCOM for emery (Test Minute: {minute // 60}:{minute % 60:02d})...")
    env = dict(
        os.environ,
        TERCOM_TEST_MINUTE=str(minute),
        TERCOM_TEST_HEADING=str(heading),
        TERCOM_TEST_STEPS=str(steps),
        TERCOM_TEST_BATTERY=str(battery),
        TERCOM_TEST_ALT=str(alt),
    )
    r = run_cmd(["pebble", "build"], env=env)
    if r.returncode != 0:
        print("Build failed:\n" + r.stdout + "\n" + r.stderr)
        sys.exit(1)
    print("✓ Pebble build succeeded.")


def install_and_capture(screenshot_name: str = "emery_tercom"):
    OUT.mkdir(parents=True, exist_ok=True)
    STORE.mkdir(parents=True, exist_ok=True)

    print("Booting emery emulator and installing...")
    r = run_cmd(["pebble", "install", "--emulator", "emery"], timeout=180)
    print(r.stdout.strip())

    # Wait for emulator to fully render frame
    time.sleep(6)

    # Per _PEBBLE skill: second install guarantees not displaying stale cache
    print("Second install pass to verify active bundle...")
    r2 = run_cmd(["pebble", "install", "--emulator", "emery"], timeout=120)
    print(r2.stdout.strip())
    time.sleep(3)

    out_path = OUT / f"{screenshot_name}.png"
    store_path = STORE / f"{screenshot_name}.png"

    print(f"Capturing native screenshot: {out_path}...")
    run_cmd(["pebble", "screenshot", "--emulator", "emery", "--no-open", "--no-correction", str(out_path)], timeout=45)

    if out_path.exists() and out_path.stat().st_size > 0:
        import shutil
        shutil.copyfile(out_path, store_path)
        print(f"✓ Screenshot saved to {out_path} and copied to {store_path} ({out_path.stat().st_size} bytes).")
    else:
        print(f"ERROR: Screenshot failed to generate at {out_path}")
        sys.exit(1)


def main():
    clean_emulator()
    build_with_fixtures()
    install_and_capture("emery_tercom")
    print("Emulator check complete.")


if __name__ == "__main__":
    main()
