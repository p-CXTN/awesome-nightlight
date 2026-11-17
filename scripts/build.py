"""Compile and package firmware using an installed Arduino ESP32 core."""
import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--cli', default='arduino-cli', help='Arduino CLI executable')
parser.add_argument('--version', help='Explicit MAJOR.MINOR.PATCH release version')
parser.add_argument('--images-dir', type=Path, default=ROOT / 'firmware/images', help='Release destination')
parser.add_argument('--build-property', action='append', default=[], help='Additional Arduino build property')
args = parser.parse_args()
output = ROOT / '.nightlight-build/out'
command = [args.cli, 'compile', '--fqbn',
           'esp32:esp32:esp32c3:CDCOnBoot=cdc,FlashSize=4M,PartitionScheme=default',
           '--warnings', 'default', '--build-path', str(ROOT / '.nightlight-build/build'),
           '--output-dir', str(output)]
for prop in args.build_property:
    command += ['--build-property', prop]
subprocess.run(command + [str(ROOT / 'firmware/NightlightRevK')], check=True)
package = [sys.executable, str(ROOT / 'firmware/package_release.py'), '--build-dir', str(output),
           '--images-dir', str(args.images_dir)]
package += ['--version', args.version or (ROOT / 'firmware/VERSION').read_text().strip()]
subprocess.run(package, check=True)
