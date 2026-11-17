"""Package matching OTA/USB images and retain immutable versioned releases."""

import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re

PROJECT = Path(__file__).resolve().parents[1]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def validate(app, usb):
    if not (app[:1] == b'\xe9' and app[32:36] == bytes.fromhex('3254cdab')):
        raise ValueError('Invalid OTA application header')
    if len(app) >= 0x140000 or len(usb) != 0x400000:
        raise ValueError('Image exceeds OTA slot or USB image is not 4 MB')
    if usb[0x10000:0x10000 + len(app)] != app:
        raise ValueError('USB image does not contain the matching OTA application')
    if usb[0x9000:0xe000] != b'\xff' * 0x5000:
        raise ValueError('USB image does not clear NVS')


def write_release(folder, version, timestamp, app, usb):
    folder.mkdir(parents=True, exist_ok=False)
    stamp = timestamp.strftime('%Y%m%dT%H%M%SZ')
    prefix = f'NightlightRevK-v{version}-{stamp}-ESP32C3'
    files = {}
    for kind, data in [('OTA', app), ('USB', usb)]:
        name = f'{prefix}-{kind}.bin'
        (folder / name).write_bytes(data)
        files[kind.lower()] = {'name': name, 'sha256': digest(data), 'bytes': len(data)}
    manifest = {
        'version': version,
        'build_time_utc': timestamp.isoformat(),
        'build_time_source': 'application binary modification time',
        'board': 'ESP32-C3 SuperMini / Rev K / 4 MB',
        'files': files,
    }
    (folder / 'release.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    (folder / 'SHA256SUMS').write_text(''.join(
        f"{entry['sha256']}  {entry['name']}\n" for entry in files.values()), encoding='ascii')
    return manifest


def read_release(folder):
    manifest = json.loads((folder / 'release.json').read_text(encoding='utf-8'))
    for entry in manifest['files'].values():
        name = entry['name']
        if Path(name).name != name or digest((folder / name).read_bytes()) != entry['sha256']:
            raise ValueError(f'Invalid archived image: {name}')
    return manifest


def move_release(source, destination):
    manifest = read_release(source)
    names = [entry['name'] for entry in manifest['files'].values()] + ['SHA256SUMS', 'release.json']
    destination.mkdir(parents=True, exist_ok=True)
    if any((destination / name).exists() for name in names):
        raise ValueError('Release destination contains conflicting files')
    moved = []
    try:
        for name in names:
            (source / name).rename(destination / name)
            moved.append(name)
    except OSError:
        for name in reversed(moved):
            (destination / name).rename(source / name)
        raise


def package(build_dir, images_dir, version=None):
    build_dir, images_dir = Path(build_dir), Path(images_dir)
    app_path = build_dir / 'NightlightRevK.ino.bin'
    app = app_path.read_bytes()
    usb = (build_dir / 'NightlightRevK.ino.merged.bin').read_bytes()
    validate(app, usb)
    timestamp = datetime.fromtimestamp(app_path.stat().st_mtime, timezone.utc).replace(microsecond=0)
    latest = images_dir
    archive = images_dir / 'archive'
    previous_layout = images_dir / 'latest'
    if previous_layout.exists():
        move_release(previous_layout, images_dir)
        previous_layout.rmdir()
    current = read_release(latest) if (latest / 'release.json').exists() else None
    if version is not None and not re.fullmatch(r'\d+\.\d+\.\d+', version):
        raise ValueError('Version must have the form MAJOR.MINOR.PATCH')
    if current and current['files']['ota']['sha256'] == digest(app) and current['files']['usb']['sha256'] == digest(usb):
        if version is None or version == current['version']:
            return latest, current
    if version is None:
        numbers = [int(n) for n in current['version'].split('.')] if current else [1, 0, -1]
        numbers[2] += 1
        version = '.'.join(map(str, numbers))
    if current and tuple(map(int, version.split('.'))) <= tuple(map(int, current['version'].split('.'))):
        raise ValueError('A changed release must have a higher version')
    if (archive / f'v{version}').exists():
        raise ValueError(f'Version {version} already exists in the archive')
    old_destination = archive / f"v{current['version']}" if current else None
    if old_destination and old_destination.exists():
        raise ValueError('Previous version already exists in archive; refusing to overwrite')

    # Preserve the unversioned pair from before versioned packaging was introduced.
    legacy_app = images_dir / 'ota.bin'
    legacy_usb = images_dir / 'NightlightRevK-USB.bin'
    if legacy_app.exists() or legacy_usb.exists():
        old_app, old_usb = legacy_app.read_bytes(), legacy_usb.read_bytes()
        validate(old_app, old_usb)
        legacy = archive / 'v0.0.0-legacy'
        if legacy.exists():
            old = read_release(legacy)
            if old['files']['ota']['sha256'] != digest(old_app) or old['files']['usb']['sha256'] != digest(old_usb):
                raise ValueError('Different legacy release already archived')
        else:
            legacy_time = datetime.fromtimestamp(legacy_app.stat().st_mtime, timezone.utc).replace(microsecond=0)
            write_release(legacy, '0.0.0-legacy', legacy_time, old_app, old_usb)
            metadata = json.loads((legacy / 'release.json').read_text(encoding='utf-8'))
            metadata['build_time_source'] = 'legacy image modification time; original build time unknown'
            (legacy / 'release.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8')
        if not current:
            (images_dir / 'SHA256SUMS').unlink(missing_ok=True)

    archive.mkdir(parents=True, exist_ok=True)
    stage = images_dir / f'.staging-v{version}'
    manifest = write_release(stage, version, timestamp, app, usb)
    read_release(stage)
    if current:
        move_release(latest, old_destination)
    try:
        move_release(stage, latest)
    except (OSError, ValueError):
        if current:
            move_release(old_destination, latest)
            old_destination.rmdir()
        raise
    stage.rmdir()
    for path in [legacy_app, legacy_usb]:
        if path.exists():
            path.unlink()
    return latest, manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version', help='Explicit MAJOR.MINOR.PATCH; otherwise increment patch for changed images')
    parser.add_argument('--build-dir', type=Path, default=PROJECT / '.nightlight-build/out')
    parser.add_argument('--images-dir', type=Path, default=PROJECT / 'firmware/images')
    args = parser.parse_args()
    folder, manifest = package(args.build_dir, args.images_dir, args.version)
    print(f"Packaged v{manifest['version']}; matching OTA/USB images and SHA-256 checksums verified.")
    for entry in manifest['files'].values():
        print(folder / entry['name'])
