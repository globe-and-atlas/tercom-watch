"""Build committed TERCOM 1.0.1 and capture native Emery listing media."""

from __future__ import annotations

import bz2
import io
import json
import shutil
import socket
import subprocess
import tarfile
import time
import urllib.request
from pathlib import Path
from zipfile import ZipFile

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[1]
TMP = ROOT / ".tmp/listing"
STORE = ROOT / "store/listing"
SDK = Path.home() / "Library/Application Support/Pebble SDK/SDKs/4.33.1"
PUBLIC_PBW = TMP / "tercom-public-1.0.1.pbw"
PUBLIC_URL = "https://appstore-api.repebble.com/api/assets/pbw/c6f081e1607448bf92df7de4/1.0.1/12feaa91-4356-461a-bc70-4d6c99661b10.pbw"
EXPECTED_UUID = "a05c6357-104a-4090-bba4-0a1cb7606927"
RELEASE_COMMIT = "b57b27a"  # Published 1.0.1 source; master now contains 1.0.2 work.


def run(command: list[str], *, cwd: Path = ROOT, timeout: int = 120) -> str:
    result = subprocess.run(command, cwd=cwd, capture_output=True, text=True, timeout=timeout)
    if result.returncode:
        raise RuntimeError(f"Command failed: {command!r}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def pbw_info(path: Path) -> dict:
    with ZipFile(path) as archive:
        info = json.loads(archive.read("appinfo.json"))
        if "emery/pebble-app.bin" not in archive.namelist():
            raise ValueError(f"{path} has no Emery binary")
    if info["uuid"] != EXPECTED_UUID or info["targetPlatforms"] != ["emery"]:
        raise ValueError(f"{path} is not the TERCOM Emery package")
    return info


def inspect_public() -> None:
    with urllib.request.urlopen(PUBLIC_URL, timeout=30) as response:
        payload = response.read(2_000_001)
    if len(payload) > 2_000_000:
        raise ValueError("Public PBW exceeded 2 MB bound")
    PUBLIC_PBW.write_bytes(payload)
    print(f"Public PBW embedded version: {pbw_info(PUBLIC_PBW)['versionLabel']}", flush=True)


def build_committed_release() -> Path:
    clean = TMP / "committed-release"
    if clean.exists():
        shutil.rmtree(clean)
    clean.mkdir(parents=True)
    archive = subprocess.run(["git", "archive", RELEASE_COMMIT], cwd=ROOT, capture_output=True, check=True)
    with tarfile.open(fileobj=io.BytesIO(archive.stdout)) as tar:
        tar.extractall(clean, filter="data")
    face = clean / "watchface"
    print(run(["pebble", "build"], cwd=face, timeout=180), flush=True)
    pbw = face / "build/watchface.pbw"
    if pbw_info(pbw)["versionLabel"] != "1.0.1":
        raise ValueError("Committed source did not produce TERCOM 1.0.1")
    return pbw


def unused_port() -> int:
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def wait_for_port(port: int, process: subprocess.Popen[bytes]) -> None:
    for _ in range(100):
        if process.poll() is not None:
            raise RuntimeError(f"Private QEMU exited with status {process.returncode}")
        with socket.socket() as sock:
            sock.settimeout(0.2)
            if sock.connect_ex(("127.0.0.1", port)) == 0:
                return
        time.sleep(0.2)
    raise TimeoutError("Private QEMU did not open its serial port")


def capture(name: str, port: int) -> Path:
    path = TMP / f"{name}.png"
    run(["pebble", "screenshot", "--qemu", f"localhost:{port}", "--platform", "emery", "--no-open", "--no-correction", str(path)], timeout=45)
    with Image.open(path) as image:
        if image.size != (200, 228):
            raise ValueError(f"{name} dimensions are {image.size}, expected 200x228")
    print(f"Captured {name}", flush=True)
    return path


def private_emulator_captures(pbw: Path) -> list[Path]:
    with bz2.open(SDK / "sdk-core/pebble/emery/qemu/qemu_spi_flash.bin.bz2", "rb") as src:
        (TMP / "private_spi_flash.bin").write_bytes(src.read())
    serial, second_serial, gdb, monitor = [unused_port() for _ in range(4)]
    command = [
        str(SDK / "toolchain/bin/qemu-pebble"), "-rtc", "base=localtime", "-serial", "null",
        "-serial", f"tcp::{serial},server=on,wait=off", "-serial", f"tcp::{second_serial},server=on,wait=off",
        "-kernel", str(SDK / "sdk-core/pebble/emery/qemu/qemu_micro_flash.bin"),
        "-gdb", f"tcp::{gdb},server=on,wait=off", "-monitor", f"tcp::{monitor},server=on,wait=off",
        "-machine", "pebble-emery", "-cpu", "cortex-m33",
        "-drive", f"if=mtd,format=raw,file={TMP / 'private_spi_flash.bin'}",
        "-audio", "driver=none,id=audio0", "-display", "none",
    ]
    with (TMP / "private-qemu.log").open("wb") as log:
        process = subprocess.Popen(command, cwd=TMP, stdout=log, stderr=subprocess.STDOUT)
        try:
            wait_for_port(serial, process)
            run(["pebble", "install", "--qemu", f"localhost:{serial}", "--platform", "emery", str(pbw)], timeout=180)
            time.sleep(2)
            run(["pebble", "emu-set-time", "--qemu", f"localhost:{serial}", "--platform", "emery", "10:08:00"])
            time.sleep(0.4)
            paths = [capture("tercom_1008", serial)]
            run(["pebble", "emu-set-time", "--qemu", f"localhost:{serial}", "--platform", "emery", "10:17:00"])
            time.sleep(0.4)
            paths.append(capture("tercom_1017", serial))
            run(["pebble", "emu-tap", "--qemu", f"localhost:{serial}", "--platform", "emery"])
            for index in range(5):
                paths.append(capture(f"tercom_flight_{index}", serial))
            return paths
        finally:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=5)


def prepare_media(paths: list[Path]) -> None:
    STORE.mkdir(parents=True, exist_ok=True)
    for index, src in enumerate([paths[0], paths[1], paths[-1]], start=1):
        shutil.copyfile(src, STORE / f"emery_{index:02d}.png")
    frames = [Image.open(path).convert("RGB") for path in paths]
    distinct = sum(ImageChops.difference(frames[0], frame).getbbox() is not None for frame in frames[1:])
    if distinct < 2:
        raise ValueError("Native capture sequence has fewer than three distinct frames")
    gif = STORE / "emery_00_preview.gif"
    frames[0].save(gif, save_all=True, append_images=frames[1:], duration=[850, 600, 170, 170, 170, 170, 1100], loop=0, optimize=True)
    with Image.open(gif) as image:
        if image.n_frames < 3 or image.size != (200, 228):
            raise ValueError("Listing GIF failed frame or dimension validation")
    if gif.stat().st_size > 4_400_000:
        raise ValueError("Listing GIF exceeds 4.4 MB")
    print(f"Listing media: {STORE}; GIF frames={len(frames)}; distinct={distinct}", flush=True)


def main() -> None:
    TMP.mkdir(parents=True, exist_ok=True)
    inspect_public()
    release = build_committed_release()
    prepare_media(private_emulator_captures(release))


if __name__ == "__main__":
    main()
