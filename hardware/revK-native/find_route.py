"""Search independent routing candidates and retain the most complete result."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
import json, os, random, subprocess, sys

ROOT = Path(__file__).resolve().parent
base = ['RADAR_OUT', 'RADAR_TX', 'RADAR_RX', 'I2C_SDA', 'I2C_SCL'] + [
    f'LED{i}_{suffix}' for suffix in ['PWM', 'A', 'GATE', 'RETURN'] for i in [1, 2, 3]
] + ['+3V3', '+5V', 'GND']
local = [f'LED{i}_{suffix}' for suffix in ['GATE', 'RETURN', 'A'] for i in [1, 2, 3]]
refine='--refine' in sys.argv
anchor=json.loads((ROOT/'routing-order.json').read_text()) if refine else []

def candidate(seed):
    folder = ROOT / '_routing' / str(seed)
    folder.mkdir(parents=True, exist_ok=True)
    for name in ['route.py', 'design-model.json']:
        (folder / name).write_bytes((ROOT / name).read_bytes())
    order = base.copy()
    random.Random(seed).shuffle(order)
    if seed % 3 == 0:
        order = local + [n for n in order if n not in local]
    elif seed % 3 == 1:
        order = ['GND', '+5V', '+3V3'] + [n for n in order if n not in ['GND', '+5V', '+3V3']]
    if refine:
        groups=[['LED2_PWM','LED3_PWM','I2C_SCL','GND'],['LED2_PWM','LED3_PWM'],['LED2_PWM','LED3_PWM','I2C_SCL'],['GND'],['LED1_PWM','LED2_PWM','LED3_PWM'],['I2C_SCL']]
        group=groups[(seed//12)%len(groups)].copy()
        if seed%2:group.reverse()
        rest=[n for n in anchor if n not in group]
        index=seed%12
        order=rest[:index]+group+rest[index:]
    result = subprocess.run([sys.executable, str(folder / 'route.py')],
                            env=dict(os.environ, ROUTE_ORDER=json.dumps(order)),
                            capture_output=True, text=True)
    (folder / 'run.log').write_text(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(result.stderr)
    data = json.loads((folder / 'routes.json').read_text())
    return seed, order, data

best = (1000, 1000)
for start in range(0, 72 if refine else 32, 4):
    with ThreadPoolExecutor(max_workers=4) as pool:
        for job in as_completed([pool.submit(candidate, seed) for seed in range(start, start + 4)]):
            seed, order, data = job.result()
            score = (len(data['unrouted']), len(data['vias']))
            print('Candidate', seed, 'unrouted', data['unrouted'], 'vias', score[1], flush=True)
            if score < best:
                best = score
                (ROOT / 'routes.json').write_text(json.dumps(data, indent=2))
                (ROOT / 'routing-order.json').write_text(json.dumps(order, indent=2))
    if best[0] == 0:
        break
if best[0]:
    raise SystemExit(f'No complete route found: {best[0]} remaining connections')
