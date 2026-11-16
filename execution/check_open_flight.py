"""Verify the Opening flight setting on the Emery emulator (launcher round trip)."""
import bz2, subprocess, sys, time
from pathlib import Path
from PIL import Image, ImageChops
ROOT = Path.cwd()  # .tmp is a symlink off-project
sys.path.insert(0, str(ROOT / 'execution'))
from prepare_listing_media import SDK, run, unused_port, wait_for_port

UUID = 'a05c6357-104a-4090-bba4-0a1cb7606927'
pbw, out = Path(sys.argv[1]), Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
fw = SDK / 'sdk-core/pebble/emery/qemu'
flash = out / 'flash.bin'
with bz2.open(fw / 'qemu_spi_flash.bin.bz2', 'rb') as src: flash.write_bytes(src.read())
serial, second, monitor = [unused_port() for _ in range(3)]
cmd = [str(SDK / 'toolchain/bin/qemu-pebble'), '-rtc', 'base=localtime', '-serial', 'null',
       '-serial', f'tcp::{serial},server=on,wait=off', '-serial', f'tcp::{second},server=on,wait=off',
       '-kernel', str(fw / 'qemu_micro_flash.bin'), '-monitor', f'tcp::{monitor},server=on,wait=off',
       '-machine', 'pebble-emery', '-cpu', 'cortex-m33', '-drive', f'if=mtd,format=raw,file={flash}', '-display', 'none']
base = ['--qemu', f'localhost:{serial}', '--platform', 'emery']
def send(values):
    run(['pebble', 'send-app-message', *base, '--app-uuid', UUID, '--int', *[f'{10000+k}={v}' for k, v in values.items()]])
def shot(name):
    p = out / f'{name}.png'
    run(['pebble', 'screenshot', *base, '--no-open', '--no-correction', str(p)], timeout=30)
    with Image.open(p) as s: return s.convert('RGB')
def round_trip(tag):
    run(['pebble', 'emu-button', *base, 'click', 'select']); time.sleep(1.5)
    run(['pebble', 'emu-button', *base, 'click', 'back']); time.sleep(.3)
    a = shot(f'{tag}-a'); time.sleep(.3); b = shot(f'{tag}-b')
    return ImageChops.difference(a, b).getbbox() is not None
with (out / 'qemu.log').open('wb') as log:
    proc = subprocess.Popen(cmd, stdout=log, stderr=subprocess.STDOUT)
    try:
        wait_for_port(serial, proc)
        run(['pebble', 'emu-set-time', *base, '23:59:00'])
        run(['pebble', 'install', *base, str(pbw)], timeout=100); time.sleep(6)
        on = round_trip('on')                     # default: opening flight On
        send({10: 0}); time.sleep(1)              # OPEN_FLIGHT = Off
        off = round_trip('off')
        rest = shot('off-rest'); run(['pebble', 'emu-tap', *base]); time.sleep(.4)
        tap = ImageChops.difference(rest, shot('off-tap')).getbbox() is not None
        print({'opening_flight_on_moves': on, 'opening_flight_off_moves': off, 'tap_flight_with_off': tap})
        assert on and not off and tap
    finally:
        proc.terminate(); proc.wait(timeout=12)
