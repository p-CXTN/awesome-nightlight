"""Route the provisional two-layer model with clearance-aware grid search."""
import json,math,heapq,time,os,random
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw
from scipy.ndimage import distance_transform_edt, distance_transform_cdt

ROOT=Path(__file__).resolve().parent
m=json.loads((ROOT/'design-model.json').read_text())
S=20; W,H=[int(v*S)+1 for v in m['board']]
tracks=[];vias=[]
# Reserve short escape stubs before global routing so dense front pads retain
# a usable connection to the rear layer. Vias sit beside, not inside, the lands.
for ref,num,dx,dy in [('R3','1',-1.25,0),('R7','1',-1.25,0),('R10','1',-1.25,0),('U3','3',1.5,0),('R8','2',0,1.2),('U1','8',2.6,0),('U1','7',2.6,0),('U1','9',2.9,0),('U1','10',6.9,0),('U1','14',2.9,0)]:
    p=next(p for p in m['pads'] if p['ref']==ref and p['num']==num)
    x,y=round((p['x']+dx)*S)/S,round((p['y']+dy)*S)/S
    tracks.append(dict(net=p['net'],layer=1,width=.3 if p['net']=='GND' else .25,points=[(p['x'],p['y']),(x,y)]))
    vias.append(dict(x=x,y=y,diameter=.7,drill=.3,net=p['net']))
# Escape the radar input beyond the centered connector's shell band.
p=next(p for p in m['pads'] if p['ref']=='U1' and p['num']=='11')
tracks.append(dict(net=p['net'],layer=1,width=.25,points=[(p['x'],p['y']),(31.3,p['y']),(31.3,29.3)]))
vias.append(dict(x=31.3,y=29.3,diameter=.7,drill=.3,net=p['net']))

def raster(net):
    imgs=[Image.new('1',(W,H)) for _ in range(2)]; ds=[ImageDraw.Draw(i) for i in imgs]
    for d in ds:
        d.rectangle((0,0,W-1,H-1),outline=1,width=3)
        for x1,y1,x2,y2 in m['keepouts']: d.rectangle((x1*S,y1*S,x2*S,y2*S),fill=1)
        for x,y,diam in m['holes']:
            r=2.2;d.ellipse(((x-r)*S,(y-r)*S,(x+r)*S,(y+r)*S),fill=1)
    for reservation in m.get('layer_keepouts',[]):
        x1,y1,x2,y2=reservation['rect']
        ds[reservation['layer']-1].rectangle((x1*S,y1*S,x2*S,y2*S),fill=1)
    if net!='GND':
        for x1,y1,x2,y2 in m.get('shell_contact_bands',[]):
            ds[1].rectangle((x1*S,y1*S,x2*S,y2*S),fill=1)
    for p in m['pads']:
        if p['net']==net: continue
        layers=[0,1] if p['layer']==11 else [p['layer']-1]
        x,y,w,h=[p[k]*S for k in ['x','y','w','h']]
        for l in layers:
            box=(x-w/2,y-h/2,x+w/2,y+h/2)
            if p['shape']=='ELLIPSE': ds[l].ellipse(box,fill=1)
            else: ds[l].rectangle(box,fill=1)
    for r in tracks:
        if r['net']==net: continue
        d=ds[r['layer']-1]; pts=[(round(x*S),round(y*S)) for x,y in r['points']]; radius=r['width']*S/2
        d.line(pts,fill=1,width=math.ceil(r['width']*S))
        for x,y in pts: d.ellipse((x-radius,y-radius,x+radius,y+radius),fill=1)
    for v in vias:
        if v['net']==net: continue
        x,y,r=v['x']*S,v['y']*S,v['diameter']*S/2
        for d in ds:d.ellipse((x-r,y-r,x+r,y+r),fill=1)
    return np.stack([distance_transform_edt(~np.asarray(i,dtype=bool))/S for i in imgs])

def states(p):
    return [(l,round(p['y']*S),round(p['x']*S)) for l in ([0,1] if p['layer']==11 else [p['layer']-1])]

def search(start,targets,dist,width,net=None):
    limit=width/2+.2+.05
    allowed=dist>limit
    viaok=(dist[0]>.35+.2+.05)&(dist[1]>.35+.2+.05)
    yy,xx=np.mgrid[0:H,0:W]
    # Drill spacing applies even when copper belongs to the same net.
    for p in m['pads']:
        if p['drill']:
            viaok &= np.hypot(xx/S-p['x'],yy/S-p['y']) >= .15+p['drill']/2+.35
    for x,y,diam in m['holes']:
        viaok &= np.hypot(xx/S-x,yy/S-y) >= .15+diam/2+.35
    for v in vias:
        distance=np.hypot(xx/S-v['x'],yy/S-v['y'])
        viaok &= (distance >= .15+v['drill']/2+.35) | (distance < 1e-8)
    # Reusing an existing same-net via adds no drill or annular copper.
    for v in vias:
        if v['net']==net:viaok[round(v['y']*S),round(v['x']*S)]=True
    targetset=set(targets);tx=np.array([p[2] for p in targets]);ty=np.array([p[1] for p in targets])
    goal_mask=np.ones((H,W),dtype=bool)
    for _,ty,tx in targets:goal_mask[ty,tx]=False
    goal_distance=distance_transform_cdt(goal_mask,metric='taxicab')
    def heuristic(node):
        l,y,x=node
        return int(goal_distance[y,x])
    q=[];cost={};prev={};serial=0
    for node in start:
        cost[node]=0;heapq.heappush(q,(heuristic(node),serial,node));serial+=1
    while q:
        score,_,node=heapq.heappop(q); g=cost[node]
        if score>g+heuristic(node)+1e-6:continue
        if node in targetset:
            path=[node]
            while node in prev:node=prev[node];path.append(node)
            return list(reversed(path))
        l,y,x=node
        neighbors=[((l,y,x+1),1),((l,y,x-1),1),((l,y+1,x),1),((l,y-1,x),1)]
        if viaok[y,x]: neighbors.append(((1-l,y,x),50))
        for nxt,c in neighbors:
            nl,ny,nx=nxt
            if not(0<=nx<W and 0<=ny<H) or not allowed[nl,ny,nx]:continue
            ng=g+c
            if ng<cost.get(nxt,1e30):
                cost[nxt]=ng;prev[nxt]=node;heapq.heappush(q,(ng+heuristic(nxt),serial,nxt));serial+=1
    return None

def commit(path,p1,p2,net,width):
    run=[];layer=path[0][0]
    def save(points,l):
        compact=[]
        for p in points:
            if compact and p==compact[-1]:continue
            if len(compact)>1:
                a,b=compact[-2:]
                if abs((b[0]-a[0])*(p[1]-b[1])-(b[1]-a[1])*(p[0]-b[0]))<1e-8:
                    compact[-1]=p;continue
            compact.append(p)
        if len(compact)>1: tracks.append(dict(net=net,layer=l+1,width=width,points=compact))
    run=[(p1['x'],p1['y'])]
    for l,y,x in path:
        p=(x/S,y/S)
        if l!=layer:
            save(run,layer);run=[p];layer=l
            if not any(v['x']==p[0] and v['y']==p[1] and v['net']==net for v in vias):vias.append(dict(x=p[0],y=p[1],diameter=.7,drill=.3,net=net))
        run.append(p)
    run.append((p2['x'],p2['y']));save(run,layer)

nets={}
for p in m['pads']:
    if p['net']:nets.setdefault(p['net'],[]).append(p)
# Power first, then short local nets, then longer signal nets.
order=['RADAR_OUT','RADAR_TX','RADAR_RX','I2C_SDA','I2C_SCL']+[f'LED{i}_{suffix}' for suffix in ['PWM','A','GATE','RETURN'] for i in [1,2,3]]+['+3V3','+5V','GND']
seed=int(os.environ.get('ROUTE_SEED','22'));random.Random(seed).shuffle(order)
if seed%2==0:order=['GND','+5V','+3V3']+[n for n in order if n not in ['GND','+5V','+3V3']]
if 'ROUTE_ORDER' in os.environ:order=json.loads(os.environ['ROUTE_ORDER'])
elif 'ROUTE_SEED' not in os.environ and (ROOT/'routing-order.json').exists():
    order=json.loads((ROOT/'routing-order.json').read_text())
unrouted=[]
for net in order:
    pp=nets[net];connected=[pp[0]];remaining=pp[1:];width=.5 if net in ['+5V','GND'] else .25
    tree=set(states(pp[0]))
    dist=raster(net)
    while remaining:
        high=[i for i,p in enumerate(remaining) if p['ref'][0] in 'JUC'] if net in ['+5V','GND'] else []
        candidates=sorted((min(math.hypot(a['x']-b['x'],a['y']-b['y']) for b in connected),i) for i,a in enumerate(remaining) if not high or i in high)
        found=False
        for _,i in candidates:
            a=remaining[i]
            branch_width=.3 if net in ['+5V','GND'] and a['ref'][0] in 'RQD' else width
            path=search(states(a),list(tree),dist,branch_width,net)
            if path:
                end=path[-1];target={'x':end[2]/S,'y':end[1]/S}
                commit(path,a,target,net,branch_width);tree.update(path)
                connected.append(a);remaining.pop(i);found=True;break
        if not found:
            unrouted.extend(f'{net}: {p["ref"]}.{p["num"]}' for p in remaining);break
    print(net,'connected',len(connected),'/',len(pp),flush=True)
result=dict(tracks=tracks,vias=vias,unrouted=unrouted,grid_mm=1/S,clearance_mm=.2)
(ROOT/'routes.json').write_text(json.dumps(result,indent=2))
print('Tracks',len(tracks),'vias',len(vias),'unrouted',unrouted,flush=True)
