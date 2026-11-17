"""Package the checked-in hardware documents and fabrication files for a release."""
import hashlib
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
hardware = ROOT / 'hardware/revK-native'
output = ROOT / 'firmware/images'
output.mkdir(parents=True, exist_ok=True)
archives = {
    'Nightlight-RevK-JLCEDA-Standard.zip': [
        (hardware / name, name) for name in ('Nightlight-PCB.json', 'Nightlight-Schematic.json')],
}
for name, folder in [('Gerber_Nightlight-PCB_2026-11-09.zip', 'gerber'),
                     ('Nightlight-RevK-JLC-Upload-2Layer.zip', 'jlc-upload')]:
    source = hardware / 'fabrication' / folder
    archives[name] = [(path, path.relative_to(source).as_posix())
                      for path in sorted(source.rglob('*')) if path.is_file()]
for name, files in archives.items():
    if not files:
        raise ValueError(f'No input files for {name}')
    with zipfile.ZipFile(output / name, 'w', zipfile.ZIP_DEFLATED) as archive:
        for path, member in files:
            archive.write(path, member)
checksums = ''.join(f'{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n'
                    for path in sorted(output.iterdir()) if path.suffix in ('.bin', '.zip'))
(output / 'SHA256SUMS').write_text(checksums, encoding='ascii')
