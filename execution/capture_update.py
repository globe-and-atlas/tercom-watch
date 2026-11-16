"""Replay TERCOM settings and motion in a private Emery emulator."""

from pathlib import Path
import argparse
import bz2
import subprocess
import time

from PIL import Image, ImageChops
from prepare_listing_media import ROOT, SDK, run, unused_port, wait_for_port

OUT = ROOT / '.tmp/update-1.1.0'
MEDIA = ROOT / 'store/preview-1.1.0'
UUID = 'a05c6357-104a-4090-bba4-0a1cb7606927'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--production', action='store_true', help='Verify live clock instead of fixed-time media')
    args = parser.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    MEDIA.mkdir(parents=True, exist_ok=True)
    with bz2.open(SDK / 'sdk-core/pebble/emery/qemu/qemu_spi_flash.bin.bz2', 'rb') as source:
        (OUT / 'flash.bin').write_bytes(source.read())
    serial, second, monitor = [unused_port() for _ in range(3)]
    command = [str(SDK / 'toolchain/bin/qemu-pebble'), '-rtc', 'base=localtime',
        '-serial', 'null', '-serial', f'tcp::{serial},server=on,wait=off',
        '-serial', f'tcp::{second},server=on,wait=off',
        '-kernel', str(SDK / 'sdk-core/pebble/emery/qemu/qemu_micro_flash.bin'),
        '-monitor', f'tcp::{monitor},server=on,wait=off',
        '-machine', 'pebble-emery', '-cpu', 'cortex-m33',
        '-drive', f'if=mtd,format=raw,file={OUT / "flash.bin"}',
        '-audio', 'driver=none,id=audio0', '-display', 'none']
    base = ['--qemu', f'localhost:{serial}', '--platform', 'emery']

    def send(values):
        run(['pebble', 'send-app-message', *base, '--app-uuid', UUID,
             '--int', *[f'{10000 + key}={value}' for key, value in values.items()]])

    def capture(name, durable=False):
        path = (MEDIA if durable else OUT) / f'{name}.png'
        run(['pebble', 'screenshot', *base, '--no-open', '--no-correction', str(path)])
        image = Image.open(path).convert('RGB')
        assert image.size == (200, 228)
        print(name, flush=True)
        return image

    def terrain_equal(a, b):
        return ImageChops.difference(a.crop((0, 110, 200, 210)), b.crop((0, 110, 200, 210))).getbbox() is None

    with (OUT / 'qemu.log').open('wb') as log:
        process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT)
        try:
            wait_for_port(serial, process)
            run(['pebble', 'install', *base, str(ROOT / 'watchface/build/watchface.pbw')], timeout=180)
            time.sleep(6)
            if args.production:
                send({0:0, 1:0, 3:0, 5:0, 7:0, 8:0})
                run(['pebble', 'emu-set-time', *base, '20:16:00'])
                time.sleep(.4)
                first = capture('production-live-a')
                run(['pebble', 'emu-set-time', *base, '20:17:00'])
                time.sleep(.4)
                second = capture('production-live-b')
                assert ImageChops.difference(first.crop((44,40,156,84)), second.crop((44,40,156,84))).getbbox(), 'Clock is frozen to a fixture'
                assert terrain_equal(first, second), 'Minute update moved resting terrain'
                print('PASS: production live clock advances without moving terrain', flush=True)
                return
            send({0: 0, 1: 0, 3: 0, 5: 1, 7: 1, 8: 0})
            idle = capture('green-crags', True)
            run(['pebble', 'emu-tap', *base])
            time.sleep(.3)
            assert terrain_equal(idle, capture('off-after-tap'))
            send({1: 1})
            run(['pebble', 'emu-tap', *base])
            active = capture('short-active')
            assert not terrain_equal(idle, active)
            time.sleep(2.3)
            assert terrain_equal(idle, capture('short-restored'))
            send({1: 2})
            run(['pebble', 'emu-tap', *base])
            run(['pebble', 'emu-accel', *base, 'tilt-right'])
            time.sleep(.6)
            tilted = capture('standard-tilt')
            assert ImageChops.difference(idle.crop((20, 20, 65, 100)), tilted.crop((20, 20, 65, 100))).getbbox() is not None, 'Tilt did not move the ladder'
            run(['pebble', 'emu-accel', *base, 'none'])
            time.sleep(5)
            assert terrain_equal(idle, capture('standard-restored'))
            send({1: 0, 3: 0})
            run(['pebble', 'emu-tap', *base]); run(['pebble', 'emu-tap', *base])
            assert terrain_equal(idle, capture('switch-disabled'))
            for palette, terrain, name in [(1,1,'amber-canyon'),(2,3,'ice-caldera'),(3,4,'mono-alpine')]:
                send({0:palette, 8:terrain, 7:1, 5:3})
                capture(name, True)
            for terrain, name in enumerate(['crags','canyon','dunes','caldera','alpine']):
                send({0:0, 8:terrain, 5:0, 7:1})
                capture('terrain-' + name, True)
            send({0:2, 8:3, 1:0, 5:3})
            before = capture('persist-before')
            run(['pebble', 'install', *base, str(ROOT / 'watchface/build/watchface.pbw')])
            for attempt in range(8):
                time.sleep(.5)
                after = capture('persist-after')
                if ImageChops.difference(before, after).getbbox() is None:
                    break
            else:
                raise AssertionError('Restart did not restore the exact saved presentation')
            print('PASS: Off, Short, Standard, exact restoration, switch-disable, settings persistence', flush=True)
        finally:
            process.terminate()
            process.wait(timeout=10)


if __name__ == '__main__':
    main()
