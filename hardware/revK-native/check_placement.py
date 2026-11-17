"""Check pad, edge and reservation geometry before routing."""
from pathlib import Path
import itertools, json, sys
ROOT=Path(__file__).resolve().parent
from shapely.geometry import box
m=json.loads((ROOT/'design-model.json').read_text())
errors=[]
board=box(0,0,*m['board']).buffer(-.2)
for p in m['pads']:
    p['geom']=box(p['x']-p['w']/2,p['y']-p['h']/2,p['x']+p['w']/2,p['y']+p['h']/2)
    p['layers']={1,2} if p['layer']==11 else {p['layer']}
    label=p['ref']+'.'+p['num']
    if not board.covers(p['geom']):errors.append('Board edge: '+label)
    for rect in m['keepouts']:
        if box(*rect).intersects(p['geom']):errors.append('RF reservation: '+label)
    for r in m['layer_keepouts']:
        if r['layer'] in p['layers'] and box(*r['rect']).intersects(p['geom']):errors.append('Layer reservation: '+label)
    if 2 in p['layers'] and p['net']!='GND':
        for rect in m['shell_contact_bands']:
            if box(*rect).distance(p['geom'])<.1999:errors.append('Shell rim: '+label)
for a,b in itertools.combinations(m['pads'],2):
    if a['layers']&b['layers'] and a['net']!=b['net'] and a['geom'].distance(b['geom'])<.1999:
        errors.append('Pad clearance: '+a['ref']+'.'+a['num']+' / '+b['ref']+'.'+b['num'])
print(json.dumps({'status':'FAIL' if errors else 'PASS','errors':errors},indent=2))
if errors:raise SystemExit(1)
