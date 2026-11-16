"""Capture and verify native TERCOM store media from the production PBW."""
import argparse
import bz2
import json
from pathlib import Path
import socket
import subprocess
import time
from zipfile import ZipFile

from PIL import Image, ImageChops

from prepare_listing_media import SDK, run, unused_port, wait_for_port

ROOT = Path(__file__).resolve().parents[1]
UUID = 'a05c6357-104a-4090-bba4-0a1cb7606927'
PLATFORMS = {
    'emery': ((200, 228), 'pebble-emery', 'cortex-m33'),
    'gabbro': ((260, 260), 'pebble-gabbro', 'cortex-m33'),
    'diorite': ((144, 168), 'pebble-silk-bb', 'cortex-m4'),
}


def capture_platform(pbw, platform, output, version):
    size, machine, cpu = PLATFORMS[platform]
    firmware = SDK / 'sdk-core/pebble' / platform / 'qemu'
    flash = output / f'{platform}-flash.bin'
    with bz2.open(firmware / 'qemu_spi_flash.bin.bz2', 'rb') as source:
        flash.write_bytes(source.read())
    serial, second, monitor = [unused_port() for _ in range(3)]
    flash_args = ['-drive', f'if=mtd,format=raw,file={flash}'] if platform != 'diorite' else ['-mtdblock', str(flash)]
    command = [str(SDK / 'toolchain/bin/qemu-pebble'), '-rtc', 'base=localtime',
               '-serial', 'null', '-serial', f'tcp::{serial},server=on,wait=off',
               '-serial', f'tcp::{second},server=on,wait=off',
               '-kernel', str(firmware / 'qemu_micro_flash.bin'),
               '-monitor', f'tcp::{monitor},server=on,wait=off',
               '-machine', machine, '-cpu', cpu, *flash_args, '-display', 'none']
    base = ['--qemu', f'localhost:{serial}', '--platform', platform]

    def send(values):
        run(['pebble', 'send-app-message', *base, '--app-uuid', UUID,
             '--int', *[f'{10000 + key}={value}' for key, value in values.items()]])

    def capture(name):
        path = output / f'{platform}-{name}.png'
        run(['pebble', 'screenshot', *base, '--no-open', '--no-correction', str(path)], timeout=30)
        with Image.open(path) as source:
            assert source.size == size, source.size
            return source.convert('RGB')

    with (output / f'{platform}-qemu.log').open('wb') as log:
        process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT)
        try:
            wait_for_port(serial, process)
            if platform == 'diorite':
                with socket.create_connection(('127.0.0.1', second), timeout=10) as console:
                    boot = bytearray()
                    try:
                        while not any(marker in boot for marker in (b'<SDK Home>', b'<Launcher>', b'Ready for communication')):
                            chunk = console.recv(1024)
                            if not chunk:
                                break
                            boot.extend(chunk)
                    except TimeoutError:
                        pass
                    (output / 'diorite-console.log').write_bytes(boot)
            run(['pebble', 'emu-set-time', *base, '23:59:00'])
            run(['pebble', 'install', *base, str(pbw)], timeout=100)
            time.sleep(3)
            palette = 3 if platform == 'diorite' else 0
            send({0: palette, 1: 2, 3: 0, 5: 1, 7: 1, 8: 0})
            time.sleep(.6)
            idle = capture('rest')
            run(['pebble', 'emu-tap', *base])
            run(['pebble', 'emu-accel', *base, 'tilt-right'])
            time.sleep(.4)
            flight = [capture(f'flight-{index:02d}') for index in range(9)]
            run(['pebble', 'emu-accel', *base, 'none'])
            time.sleep(4.7)
            restored = capture('restored')
            assert ImageChops.difference(idle, restored).getbbox() is None, platform
            active = []
            seen = {idle.tobytes()}
            for frame in flight:
                pixels = frame.tobytes()
                if pixels not in seen:
                    active.append(frame)
                    seen.add(pixels)
            assert len(active) >= 4, (platform, len(active))
            assert ImageChops.difference(idle, active[0]).getbbox() is not None

            gallery = ROOT / 'store' / f'listing-{version}'
            gallery.mkdir(exist_ok=True)
            animation = gallery / f'{platform}_01_flight.gif'
            frames = active + [restored]
            active[0].save(animation, save_all=True, append_images=frames[1:],
                           duration=[240] * len(active) + [800], loop=0, disposal=2, optimize=True)
            with Image.open(animation) as gif:
                assert gif.n_frames == len(frames)
                for index, frame in enumerate(frames):
                    gif.seek(index)
                    assert gif.convert('RGB').tobytes() == frame.tobytes()
            for index, (name, frame) in enumerate((('crags', idle), ('bank', active[len(active) // 2])), 2):
                frame.save(gallery / f'{platform}_{index:02d}_{name}.gif', optimize=True)
            for index, (terrain, color, name) in enumerate(((1, 1, 'canyon'), (4, 2, 'alpine')), 4):
                send({0: palette if platform == 'diorite' else color, 8: terrain})
                time.sleep(.25)
                frame = capture(name)
                frame.save(gallery / f'{platform}_{index:02d}_{name}.gif', optimize=True)
            result = {'platform': platform, 'size': size, 'flight_frames': len(active),
                      'exact_restoration': True}
            print(result, flush=True)
            return result
        finally:
            process.terminate()
            process.wait(timeout=12)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('pbw', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--platform', choices=PLATFORMS)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    with ZipFile(args.pbw) as archive:
        assert archive.testzip() is None
        info = json.loads(archive.read('appinfo.json'))
        assert info['uuid'] == UUID
        assert info['targetPlatforms'] == list(PLATFORMS)
    results = [capture_platform(args.pbw, platform, args.output, info['versionLabel']) for platform in ([args.platform] if args.platform else PLATFORMS)]
    (args.output / 'verification.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
