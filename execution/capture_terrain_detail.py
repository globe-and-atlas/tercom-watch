"""Capture TERCOM terrain detail and motion in a separate Emery QEMU instance."""

from __future__ import annotations

import bz2
import json
import subprocess
import time
from datetime import datetime, timedelta
from pathlib import Path
from zipfile import ZipFile

from PIL import Image, ImageChops

from prepare_listing_media import ROOT, SDK, run, unused_port, wait_for_port


OUT = ROOT / ".tmp/terrain-detail"
PBW = ROOT / "watchface/build/watchface.pbw"


def capture(name: str, port: int) -> Path:
    path = OUT / f"{name}.png"
    run(["pebble", "screenshot", "--qemu", f"localhost:{port}", "--platform", "emery", "--no-open", "--no-correction", str(path)], timeout=45)
    with Image.open(path) as image:
        if image.size != (200, 228):
            raise ValueError(f"{name}: expected Emery 200×228")
    print(f"Captured {name}", flush=True)
    return path


def main() -> None:
    with ZipFile(PBW) as archive:
        info = json.loads(archive.read("appinfo.json"))
    package = json.loads((ROOT / "watchface/package.json").read_text())
    if info["versionLabel"] != package["version"]:
        raise ValueError("Detail candidate PBW does not match package version")
    OUT.mkdir(parents=True, exist_ok=True)
    with bz2.open(SDK / "sdk-core/pebble/emery/qemu/qemu_spi_flash.bin.bz2", "rb") as src:
        (OUT / "private_spi_flash.bin").write_bytes(src.read())
    serial, second_serial, gdb, monitor = [unused_port() for _ in range(4)]
    qemu = [
        str(SDK / "toolchain/bin/qemu-pebble"), "-rtc", "base=localtime", "-serial", "null",
        "-serial", f"tcp::{serial},server=on,wait=off", "-serial", f"tcp::{second_serial},server=on,wait=off",
        "-kernel", str(SDK / "sdk-core/pebble/emery/qemu/qemu_micro_flash.bin"),
        "-gdb", f"tcp::{gdb},server=on,wait=off", "-monitor", f"tcp::{monitor},server=on,wait=off",
        "-machine", "pebble-emery", "-cpu", "cortex-m33",
        "-drive", f"if=mtd,format=raw,file={OUT / 'private_spi_flash.bin'}",
        "-audio", "driver=none,id=audio0", "-display", "none",
    ]
    with (OUT / "private-qemu.log").open("wb") as log:
        process = subprocess.Popen(qemu, cwd=OUT, stdout=log, stderr=subprocess.STDOUT)
        try:
            wait_for_port(serial, process)
            base = ["--qemu", f"localhost:{serial}", "--platform", "emery"]
            run(["pebble", "install", *base, str(PBW)], timeout=180)
            time.sleep(5)
            idle = capture("crags_idle", serial)
            run(["pebble", "emu-tap", *base])
            flight_a = capture("crags_flight_a", serial)
            flight_b = capture("crags_flight_b", serial)
            run(["pebble", "emu-accel", *base, "tilt-right"])
            tilt = capture("crags_tilt", serial)
            run(["pebble", "emu-accel", *base, "none"])
            time.sleep(5)
            terrain = (0, 110, 200, 216)
            rested = OUT / "crags_restored.png"
            with Image.open(idle) as start:
                reference = start.convert("RGB").crop(terrain)
            for attempt in range(5):
                capture("crags_restored", serial)
                with Image.open(rested) as candidate:
                    if ImageChops.difference(reference, candidate.convert("RGB").crop(terrain)).getbbox() is None:
                        break
                time.sleep(0.4)
            else:
                raise ValueError("Terrain did not return to its resting view")
            next_minute = (datetime.now() + timedelta(minutes=2)).strftime("%H:%M:00")
            run(["pebble", "emu-set-time", *base, next_minute])
            time.sleep(0.5)
            after_time = capture("crags_time_advanced", serial)
            with Image.open(rested) as before, Image.open(after_time) as after:
                if ImageChops.difference(before.convert("RGB"), after.convert("RGB")).getbbox() is None:
                    raise ValueError("Emulated time change did not redraw the watchface")
                if ImageChops.difference(before.convert("RGB").crop((0, 140, 200, 210)), after.convert("RGB").crop((0, 140, 200, 210))).getbbox() is not None:
                    raise ValueError("Time change moved the resting terrain")
            for name in ["canyon", "dunes", "caldera", "alpine"]:
                time.sleep(2.1)
                run(["pebble", "emu-tap", *base])
                run(["pebble", "emu-tap", *base])
                time.sleep(0.25)
                capture(name, serial)
            time.sleep(5)
            capture("alpine_idle", serial)
            with Image.open(idle) as start, Image.open(flight_a) as first, Image.open(flight_b) as second, Image.open(tilt) as tilted:
                if ImageChops.difference(start.convert("RGB").crop(terrain), first.convert("RGB").crop(terrain)).getbbox() is None:
                    raise ValueError("Terrain did not change after tap")
                if ImageChops.difference(first.convert("RGB").crop(terrain), second.convert("RGB").crop(terrain)).getbbox() is None:
                    raise ValueError("Terrain did not advance between active frames")
                if ImageChops.difference(second.convert("RGB").crop(terrain), tilted.convert("RGB").crop(terrain)).getbbox() is None:
                    raise ValueError("Tilt produced no visible change")
        finally:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=5)
    print(f"PASS native captures in {OUT}", flush=True)


if __name__ == "__main__":
    main()
