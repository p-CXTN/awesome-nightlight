"""Run firmware host regressions and release packaging checks."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='nightlight-tests-') as temporary:
    for source in sorted((ROOT / 'firmware/tests').glob('*.cpp')):
        binary = Path(temporary) / (source.stem + ('.exe' if os.name == 'nt' else ''))
        subprocess.run([os.environ.get('CXX', 'g++'), '-std=c++17', '-Wall', '-Wextra',
                        '-Werror', str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        print(f'PASS {source.name}', flush=True)
subprocess.run([sys.executable, str(ROOT / 'firmware/tests/release_packaging.py')], check=True)
sys.path.insert(0, str(ROOT / 'firmware'))
from package_release import read_release, validate
images = ROOT / 'firmware/images'
if (images / 'release.json').exists():
    manifest = read_release(images)
    validate(*[(images / manifest['files'][kind]['name']).read_bytes() for kind in ('ota', 'usb')])
    print('PASS current release hashes and OTA/USB pairing')
else:
    print('SKIP release image verification: build or download release assets first')
