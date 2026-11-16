"""Build a clean production TERCOM PBW without capture fixtures."""

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
from zipfile import ZipFile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    package = json.loads((ROOT / 'watchface/package.json').read_text())
    target = ROOT / 'release' / f'TERCOM-{package["version"]}.pbw'
    if args.dry_run:
        print(f'Clean production build, excluding TERCOM_TEST_* environment values → {target}')
        return
    env = {key: value for key, value in os.environ.items() if not key.startswith('TERCOM_TEST_')}
    for command in (['pebble', 'clean'], ['pebble', 'build']):
        subprocess.run(command, cwd=ROOT / 'watchface', env=env, check=True)
    source = ROOT / 'watchface/build/watchface.pbw'
    with ZipFile(source) as archive:
        info = json.loads(archive.read('appinfo.json'))
    if info['uuid'] != package['pebble']['uuid'] or info['versionLabel'] != package['version']:
        raise ValueError('Built PBW identity does not match package')
    if info['targetPlatforms'] != package['pebble']['targetPlatforms'] or info['watchapp']['watchface'] is not True:
        raise ValueError('Built PBW is missing the configured watchface platforms')
    target.parent.mkdir(exist_ok=True)
    shutil.copy2(source, target)
    print(target)


if __name__ == '__main__':
    main()
