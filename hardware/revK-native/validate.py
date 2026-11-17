"""Independent connectivity and copper-geometry checks on native file contents."""
from pathlib import Path
import sys,json,math,itertools,re
ROOT=Path(__file__).resolve().parent
from shapely.geometry import Point,LineString,box,Polygon
from shapely.ops import unary_union
m=json.loads((ROOT/'design-model.json').read_text());pcb=json.loads((ROOT/'Nightlight-PCB.json').read_text());sch=json.loads((ROOT/'Nightlight-Schematic.json').read_text())['schematics'][0]['dataStr']
errors=[];objects=[];padmap={};schmap={};pinpoints={};nc=set();labels={};segments=[];nodepoints=set();drills=[];mechanical=[]
measurements=json.loads((ROOT.parent/'measured-parts.json').read_text())
assert not any(layer.split('~')[1].startswith('Inner') for layer in pcb['layers'])
assert {layer.split('~')[1] for layer in pcb['layers'] if layer.split('~')[1] in ['TopLayer','BottomLayer']}=={'TopLayer','BottomLayer'}
assert m['board']==[52,52]
assert all(c['side']==1 for ref,c in m['components'].items() if ref!='J1')
uf=m['footprints']['U1'];sp=uf['pads']
assert [uf['w'],uf['h']]==[18,22.4]
assert sorted({p['x'] for p in sp})==[28.4,44.4]
assert abs(min(p['y'] for p in sp)-uf['y']-1.6)<1e-8
row=sorted([p for p in sp if p['x']==28.4],key=lambda p:p['y'])
assert all(abs(b['y']-a['y']-2.54)<1e-8 for a,b in zip(row,row[1:]))
assert min(p['x']-p['w']/2 for p in sp)<uf['x']
assert max(p['x']+p['w']/2 for p in sp)>uf['x']+uf['w']
gf=m['footprints']['U3']
assert [gf['w'],gf['h']]==[18.7,14.2]
gh=m['module_holes']['U3']
assert all(abs(p['x']-(gf['x']+gf['w']-1))<1e-8 for p in gh)
assert abs(min(p['y'] for p in gh)-gf['y']-2.2)<1e-8
jp={p['num']:p for p in m['footprints']['J1']['pads']}
assert set(jp)=={'1','4','5','6'}
assert jp['1']['net']=='+5V' and jp['4']['net']=='GND'
assert jp['1']['x']<jp['4']['x']
assert abs(jp['4']['x']-jp['1']['x']-7.2)<1e-8
assert jp['1']['y']==jp['4']['y']==26
assert abs((jp['1']['x']+jp['4']['x'])/2-26)<1e-8
centres=[m['footprints'][f'D{i}']['x']+1.6 for i in [1,2,3]]
assert all(abs(a-b)<1e-8 for a,b in zip(centres,[14.5,26,37.5]))
assert all(p['layer']==2 and p['drill']==0 for p in jp.values())
assert measurements['usb_a']['mounting_tab_to_contact_row_offset']==0
assert not json.loads((ROOT/'routes.json').read_text())['unrouted']
gy=sorted(gf['pads'],key=lambda p:p['y'])
assert [p['name'] for p in gy]==['VCC','GND','SCL','SDA','ADDR']
radar=sorted(m['footprints']['U2']['pads'],key=lambda p:p['x'])
assert [p['name'] for p in radar]==['TX','RX','OUT','GND','VCC']
assert len({p['y'] for p in radar})==1
assert all(abs(b['x']-a['x']-2.54)<1e-6 for a,b in zip(radar,radar[1:]))
assert all(p['drill']==0 and p['layer']==1 for ref in ['U1','U2','U3'] for p in m['footprints'][ref]['pads'])
assert [p['name'] for p in row]==['GPIO5','GPIO6','GPIO7','GPIO8','GPIO9','GPIO10','GPIO20','GPIO21']
for i,gpio in [(1,'GPIO10'),(2,'GPIO0'),(3,'GPIO1')]:
    assert next(p['net'] for p in m['components']['U1']['pins'] if p['name']==gpio)==f'LED{i}_PWM'
    assert next(p['net'] for p in m['components'][f'D{i}']['pins'] if p['name']=='K')==f'LED{i}_RETURN'
    assert next(p['net'] for p in m['components'][f'Q{i}']['pins'] if p['name']=='D')==f'LED{i}_RETURN'
def parsepoints(s):
    a=list(map(float,s.split()));return list(zip(a[::2],a[1::2]))
def addshape(s,ref=''):
    f=s.split('~');t=f[0]
    if t=='PAD':
        x,y,w,h=map(lambda s:float(s)*.254,f[2:6]);l=int(f[6]);net=f[7] or 'NC:'+ref+':'+f[8]
        radius=float(f[9])*.254
        if radius:drills.append((ref+'.'+f[8] if ref else f[12],x,y,radius))
        if f[15]=='N':
            mechanical.append((x,y,radius*2))
            return
        geom=Point(x,y).buffer(w/2,quad_segs=32) if f[1]=='ELLIPSE' else box(x-w/2,y-h/2,x+w/2,y+h/2)
        objects.append(dict(id=ref+'.'+f[8],net=net,layers={1,2} if l==11 else {l},geom=geom))
        padmap[(ref,f[8])]=f[7] or None
    elif t=='TRACK' and int(f[2]) in [1,2]:
        geom=LineString([(x*.254,y*.254) for x,y in parsepoints(f[4])]).buffer(float(f[1])*.254/2,quad_segs=16)
        objects.append(dict(id=f[5],net=f[3],layers={int(f[2])},geom=geom))
    elif t=='VIA':
        x,y,diam=map(lambda s:float(s)*.254,f[1:4]);objects.append(dict(id=f[6],net=f[4],layers={1,2},geom=Point(x,y).buffer(diam/2,quad_segs=24)))
        drills.append((f[6],x,y,float(f[5])*.254))
for shape in pcb['shape']:
    if shape.startswith('LIB~'):
        header,*children=shape.split('#@$');ref=header.split('~')[6].replace('ggeNL_','')
        for child in children:addshape(child,ref)
    else:addshape(shape)
for key,val in padmap.items():
    ref,num=key;expected=next(p['net'] for p in m['components'][ref]['pins'] if p['number']==num)
    if val!=expected:errors.append(f'Pad net mismatch: {key}')

assert len(mechanical)==4
for actual,expected in zip(mechanical,m['holes']):
    assert all(abs(a-b)<1e-5 for a,b in zip(actual,expected)),(actual,expected)
minimum_drill=100
for a,b in itertools.combinations(drills,2):
    gap=math.hypot(a[1]-b[1],a[2]-b[2])-a[3]-b[3]
    minimum_drill=min(minimum_drill,gap)
    if gap<.29999:errors.append(f'Drill clearance {gap:.4f} mm: {a[0]} / {b[0]}')
minimum=100
for a,b in itertools.combinations(objects,2):
    if a['net']==b['net'] or not (a['layers']&b['layers']):continue
    gap=a['geom'].distance(b['geom']);minimum=min(minimum,gap)
    if gap<.1999:errors.append(f'Copper clearance {gap:.4f} mm: {a["id"]} {a["net"]} / {b["id"]} {b["net"]}')
board=box(0,0,*m['board'])
for obj in objects:
    if not board.buffer(-.2).covers(obj['geom']):errors.append('Board-edge copper: '+obj['id'])
    for k in m['keepouts']:
        if box(*k).intersects(obj['geom']):errors.append('Copper keepout: '+obj['id'])
    for reservation in m['layer_keepouts']:
        if reservation['layer'] in obj['layers'] and box(*reservation['rect']).intersects(obj['geom']):
            errors.append('Layer-specific copper keepout: '+obj['id'])
    if 2 in obj['layers'] and obj['net']!='GND':
        for band in m['shell_contact_bands']:
            if box(*band).distance(obj['geom'])<.1999:
                errors.append('USB shell rim clearance: '+obj['id'])
    for x,y,diam in mechanical:
        if Point(x,y).buffer(diam/2+.2).intersects(obj['geom']):errors.append('Mounting-hole copper: '+obj['id'])
netgroups={}
for obj in objects:netgroups.setdefault(obj['net'],[]).append(obj)
for net,oo in netgroups.items():
    if net.startswith('NC:'):continue
    seen={0};todo=[0]
    while todo:
        i=todo.pop()
        for j,b in enumerate(oo):
            if j not in seen and oo[i]['layers']&b['layers'] and oo[i]['geom'].distance(b['geom'])<1e-4:
                seen.add(j);todo.append(j)
    if len(seen)!=len(oo):errors.append(f'PCB open net {net}: {len(seen)}/{len(oo)} copper objects connected')

for s in sch['shape']:
    f=s.split('~')
    if f[0]=='LIB':
        head,*children=s.split('#@$');ref=head.split('~')[6].replace('ggeNL_','')
        for p in children:
            if p.startswith('P~'):
                chunks=p.split('^^');pf=chunks[0].split('~');pt=tuple(map(float,chunks[1].split('~')));pinpoints[(ref,pf[3])]=pt;nodepoints.add(pt)
    elif f[0]=='W':
        pts=parsepoints(f[1]);nodepoints.update(pts);segments.extend(zip(pts,pts[1:]))
    elif f[0]=='N':
        pt=(float(f[1]),float(f[2]));labels.setdefault(pt,set()).add(f[5]);nodepoints.add(pt)
    elif f[0]=='J':nodepoints.add((float(f[1]),float(f[2])))
    elif f[0]=='O':nc.add((float(f[1]),float(f[2])))
parent={p:p for p in nodepoints}
def root(p):
    while p!=parent[p]:parent[p]=parent[parent[p]];p=parent[p]
    return p
for a,b in segments:
    ls=LineString([a,b]);on=[p for p in nodepoints if ls.distance(Point(p))<1e-6]
    for p in on[1:]:parent[root(p)]=root(on[0])
groupnets={}
for p,ns in labels.items():groupnets.setdefault(root(p),set()).update(ns)
for key,pt in pinpoints.items():
    actual=groupnets.get(root(pt),set());expected=padmap[key]
    if actual!=({expected} if expected else set()):errors.append(f'Schematic net {key}: {actual}, expected {expected}')
    if expected is None and pt not in nc:errors.append('Missing no-connect flag '+str(key))
    schmap['.'.join(key)]=sorted(actual)
if set(pinpoints)!=set(padmap):errors.append('Schematic/PCB pin inventory mismatch')
# Include all module lands when checking the conservative device envelope.
module_hole_clearances={}
for ref in ['U1','U2','U3']:
    f=m['footprints'][ref]
    shapes=[box(f['x'],f['y'],f['x']+f['w'],f['y']+f['h'])]
    shapes += [box(p['x']-p['w']/2,p['y']-p['h']/2,p['x']+p['w']/2,p['y']+p['h']/2) for p in f['pads']]
    # Imported device bounds can also include graphic text and outlines.
    native=next(s for s in pcb['shape'] if s.startswith('LIB~') and s.split('~')[6]=='ggeNL_'+ref)
    for child in native.split('#@$')[1:]:
        fields=child.split('~')
        if fields[0]=='TRACK':
            coords=[float(v)*.254 for v in fields[4].split()]
            shapes.append(LineString(list(zip(coords[::2],coords[1::2]))).buffer(float(fields[1])*.254/2))
        elif fields[0]=='TEXT':
            coords=[float(v)*.254 for v in re.findall(r'-?\d+(?:\.\d+)?',fields[11])]
            if coords:shapes.append(box(min(coords[::2]),min(coords[1::2]),max(coords[::2]),max(coords[1::2])).buffer(float(fields[4])*.254/2))
    envelope=unary_union(shapes).envelope
    gap=min(envelope.distance(Point(x,y).buffer(d/2,quad_segs=64)) for x,y,d in mechanical)
    module_hole_clearances[ref]=round(gap,4)
    if gap<.75:errors.append(f'Module envelope/mounting hole: {ref}: {gap:.4f} mm')
report=dict(status='PASS' if not errors else 'FAIL',component_count=len(m['components']),pad_count=len(padmap),net_count=len([n for n in netgroups if not n.startswith('NC:')]),track_count=sum(s.startswith('TRACK~') and s.split('~')[2] in ['1','2'] for s in pcb['shape']),via_count=sum(s.startswith('VIA~') for s in pcb['shape']),minimum_different_net_copper_clearance_mm=round(minimum,4),checks=['Native schematic wire/label connectivity matches every PCB pad','All connected PCB nets form continuous copper graphs','Different-net copper clearance >= 0.20 mm','No copper inside RF and trimmed-data-pin reservations','Copper-to-board edge >= 0.20 mm','Copper-to-mounting-hole edge >= 0.20 mm'],errors=errors,limitations=['Independent geometric checks, not JLCEDA native DRC','Radar edge offset and ESP antenna extent remain approximate; LED footprint retained without physical measurement','Independent checks do not validate fabrication exports'])
report['module_envelope_to_mounting_hole_clearance_mm']=module_hole_clearances
report['checks'].append('Mounting holes clear full module, land and graphic bounding envelopes by >=0.75 mm')
report['minimum_drill_edge_clearance_mm']=round(minimum_drill,4)
report['mechanical_hole_diameters_mm']=[round(d,4) for x,y,d in mechanical]
report['checks']+=['All drill pairs, including same-net vias, have >= 0.30 mm edge clearance','Four serialized non-plated mechanical holes have diameter 2.20 mm']
report['limitations'].append('Native import may require property-only Channel ID synchronization; confirm in JLCEDA')
for shape in pcb['shape']:
    if not shape.startswith('LIB~'):continue
    header,*children=shape.split('#@$')
    ref=header.split('~')[6].replace('ggeNL_','')
    names=[s.split('~')[10] for s in children if s.startswith('TEXT~N~')]
    assert names==[m['components'][ref]['value']],(ref,names)
    assert not any(s.startswith('TEXT~L~') and s.split('~')[10]==m['components'][ref]['value'] for s in children)
report['checks'].append('PCB semantic Name text equals schematic component value for every device')
report['checks'] += ['SuperMini 16 mm row spacing, 2.54 mm pitch and 1.6 mm end offset; lands extend beyond module edges', 'GY-302 measured outline, row offset and VCC end margin', 'USB polarity follows solder-end photo; 7.2 mm power span; no D+/D- copper pads', 'Router reports zero unrouted connections']
report['checks'].append('USB-A centered on both axes; LED centres equally spaced and symmetric')
report['checks'].append('Exactly two copper-layer definitions; no empty inner-copper layers')
report['checks'].append('All three module carrier connections and discretes use front SMD lands; only USB uses rear lands')
report['checks'].append('Rear non-ground copper clears USB shell contact bands by >=0.20 mm')
import hashlib
report['artifact_sha256']={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in ['Nightlight-PCB.json','Nightlight-Schematic.json','design-model.json','routes.json']}
(ROOT/'validation.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
if errors:sys.exit(1)
