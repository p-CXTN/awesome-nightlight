"""Build self-contained EasyEDA Standard schematic and PCB documents."""
from pathlib import Path
import json, uuid, math, zipfile, csv, sys
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
ROOT.mkdir(exist_ok=True)
LIB = ROOT/'libraries'; LIB.mkdir(exist_ok=True)
seq=0
def gid():
    global seq
    seq+=1
    return 'ggeNL'+str(seq)
def uid(s): return uuid.uuid5(uuid.NAMESPACE_URL,'nightlight/revK/'+s).hex
def fmt(n): return f'{n:.5f}'.rstrip('0').rstrip('.') if isinstance(n,float) else str(n)
def fields(*args): return '~'.join(fmt(x) for x in args)
def mm(v): return v/.254
def pointstr(points): return ' '.join(fmt(c) for p in points for c in p)

components={}
def component(ref,value,pins,package,side=1):
    components[ref]=dict(ref=ref,value=value,pins=[dict(number=str(n),name=name,net=net) for n,name,net in pins],package=package,side=side,id='ggeNL_'+ref,uuid=uid('symbol/'+ref),puuid=uid('footprint/'+package))
    return components[ref]
u1=[('5V','+5V'),('GND','GND'),('3V3','+3V3'),('GPIO4','I2C_SDA'),('GPIO3','RADAR_OUT'),('GPIO2',None),('GPIO1','LED3_PWM'),('GPIO0','LED2_PWM'),('GPIO5','I2C_SCL'),('GPIO6','RADAR_TX'),('GPIO7','RADAR_RX'),('GPIO8',None),('GPIO9',None),('GPIO10','LED1_PWM'),('GPIO20',None),('GPIO21',None)]
component('U1','ESP32-C3 SuperMini',[(i+1,*p) for i,p in enumerate(u1)],'SuperMini_18x22.4_SIDE_SOLDER',1)
component('U2','LD2410C',[(1,'TX','RADAR_TX'),(2,'RX','RADAR_RX'),(3,'OUT','RADAR_OUT'),(4,'GND','GND'),(5,'VCC','+5V')],'LD2410C_FRONT_FORMED_LEADS')
component('U3','GY-302 / BH1750',[(1,'VCC','+5V'),(2,'GND','GND'),(3,'SCL','I2C_SCL'),(4,'SDA','I2C_SDA'),(5,'ADDR','GND')],'GY302_FRONT_FORMED_LEADS')
component('J1','USB-A male / rear',[(1,'VBUS','+5V'),(4,'GND','GND'),(5,'SH1','GND'),(6,'SH2','GND')],'USB_A_END_SOLDER',2)
for i in [1,2,3]: component('Q'+str(i),'AO3400A',[(1,'G',f'LED{i}_GATE'),(2,'S','GND'),(3,'D',f'LED{i}_RETURN')],'SOT23_G1_S2_D3')
for ref,value,nets in [('R1','470R',['+5V','LED1_A']),('R2','470R',['+5V','LED2_A']),('R3','220R',['LED1_PWM','LED1_GATE']),('R4','10k',['LED1_GATE','GND']),('R7','220R',['LED2_PWM','LED2_GATE']),('R8','10k',['LED2_GATE','GND']),('R9','470R',['+5V','LED3_A']),('R10','220R',['LED3_PWM','LED3_GATE']),('R11','10k',['LED3_GATE','GND'])]:
    component(ref,value,[(i+1,str(i+1),n) for i,n in enumerate(nets)],'R0805',2 if ref in ['R5','R6'] else 1)
for i in range(1,6):
    component('C'+str(i),'10uF 16V' if i in [1,3] else '100nF 50V',[(1,'1','+5V'),(2,'2','GND')],'C0805',1)
for i in [1,2,3]: component('D'+str(i),'Warm white',[(1,'A','LED'+str(i)+'_A'),(2,'K',f'LED{i}_RETURN')],'LED_3.20x2.47_PROVISIONAL')

sh=[]
def text(x,y,s,mark='L',size=10): return fields('T',mark,x,y,0,'#000080','Arial',str(size)+'pt','','','','comment',s,1,'start',gid(),0)
def line(points,color='#880000',width=1): return fields('PL',pointstr(points),color,width,0,'none',gid(),0)
def rect(x,y,w,h): return fields('R',x,y,'','',w,h,'#880000',1,0,'none',gid(),0)
def wire(points): sh.append(fields('W',pointstr(points),'#008800',1,0,'none',gid(),0))
def label(x,y,net): sh.append(fields('N',x,y,0,'#008000',net,gid(),'start',x+2,y-3,'Arial','9pt',0))
def nc(x,y): sh.append(fields('O',x,y,gid(),f'M{x-3},{y-3} L{x+3},{y+3} M{x+3},{y-3} L{x-3},{y+3}','#000080',0))
def pin(x,y,number,name,dx=20,show=True,electric=0):
    return fields('P','show',electric,number,x,y,180 if dx>0 else 0,gid(),0)+'^^'+fields(x,y)+'^^'+fields(f'M {x} {y} h {dx}','#880000')+'^^'+fields(1 if show else 0,x+dx+(5 if dx>0 else -5),y+3,0,name,'start' if dx>0 else 'end','','8pt')+'^^'+fields(1 if show else 0,x+dx/2,y-4,0,number,'middle','','7pt')+'^^'+fields(0,x+dx,y)+'^^'+fields(0,f'M {x+dx} {y}')
def add_symbol(c,x,y,internal):
    attr='package`'+c['package']+'`nameAlias`Value`Value`'+c['value']+'`spicePre`'+c['ref'][0]+'`'
    header=fields('LIB',x,y,attr,0,0,c['id'],c['puuid'],c['uuid'],0,c['id'],'yes','yes','','0','')
    sh.append(header+'#@$'+'#@$'.join(internal))
    libdoc=dict(head=dict(docType='2',editorVersion='6.5.51',x=x,y=y,c_para=dict(name=c['value'],pre=c['ref'][0]+'?',package=c['package']),uuid=c['uuid']),canvas='CA~1600~1200~#FFFFFF~yes~#CCCCCC~5~1600~1200~line~5~pixel~5~0~0',shape=internal)
    (LIB/(c['ref']+'-symbol.json')).write_text(json.dumps(libdoc,indent=2))

def block(ref,x,y,split=None):
    c=components[ref]; ps=c['pins']; split=split or len(ps); h=max(split,len(ps)-split)*25+25; w=210
    internal=[rect(x,y,w,h),text(x,y-28,ref,'P'),text(x,y-12,c['value'],'N')]
    for i,p in enumerate(ps):
        left=i<split; j=i if left else i-split; px=x-20 if left else x+w+20; py=y+25+j*25
        e=4 if p['name'] in ['5V','VCC','GND','3V3','VBUS','SH1','SH2'] else (0 if ref=='J1' else 3)
        internal.append(pin(px,py,p['number'],p['name'],20 if left else -20,True,e))
        if p['net']:
            end=px-95 if left else px+30; wire([(px,py),(end,py)]); label(end,py,p['net'])
        else: nc(px,py)
    add_symbol(c,x,y,internal)

sh += [text(60,45,'USB night light - schematic',size=17),text(60,70,'Rev K engineering draft | Measured module geometry | Hand-soldered USB connector',size=10)]
block('J1',170,150)
block('U1',650,150,8)
block('U2',1170,150)
block('U3',1580,150)

# Discrete symbols use two terminals at +/-30 px. Capacitors are non-polarized.
def discrete(ref,x,y):
    c=components[ref]; internal=[text(x-15,y-15,ref,'P'),text(x-15,y+27,c['value'],'N')]
    internal += [pin(x-30,y,'1',c['pins'][0]['name'],15,False),pin(x+30,y,'2',c['pins'][1]['name'],-15,False)]
    if ref.startswith('R'): internal += [rect(x-15,y-6,30,12)]
    elif ref.startswith('C'): internal += [line([(x-15,y),(x-4,y)]),line([(x+4,y),(x+15,y)]),line([(x-4,y-12),(x-4,y+12)]),line([(x+4,y-12),(x+4,y+12)])]
    else:
        internal += [line([(x-15,y),(x-10,y)]),line([(x+10,y),(x+15,y)]),line([(x-10,y-10),(x+10,y),(x-10,y+10),(x-10,y-10)]),line([(x+10,y-10),(x+10,y+10)]),line([(x+4,y-16),(x+14,y-26),(x+8,y-24)]),line([(x+11,y-10),(x+21,y-20),(x+15,y-18)])]
    add_symbol(c,x,y,internal)
    return (x-30,y),(x+30,y)

sh += [text(60,470,'Supply decoupling',size=13),text(60,495,'C1/C2 at U1; C3/C4 at U2; C5 at U3. All capacitors 0805.',size=10)]
for i,x in enumerate([160,450,740,1030,1320],1):
    a,b=discrete('C'+str(i),x,560); wire([(a[0]-45,a[1]),a]); label(a[0]-45,a[1],'+5V'); wire([b,(b[0]+25,b[1])]); label(b[0]+25,b[1],'GND')
sh += [text(60,660,'U3 includes 4.7k SDA/SCL pull-ups to its internal regulated 3.3V rail.',size=12)]

sh += [text(60,840,'Three independent PWM LED channels',size=13)]
for i,(current,gate,pull) in enumerate([('R1','R3','R4'),('R2','R7','R8'),('R9','R10','R11')],1):
    left=60+(i-1)*620; ly=920; qx=left+480; qy=1050
    a,b=discrete(current,left+130,ly); da,db=discrete('D'+str(i),left+330,ly)
    wire([(left+30,ly),a]);label(left+30,ly,'+5V')
    wire([b,da]);label(left+220,ly,f'LED{i}_A')
    wire([db,(qx,ly),(qx,qy-40)]);label(qx,ly,f'LED{i}_RETURN')
    c=components['Q'+str(i)];x=qx;y=qy
    internal=[text(x+30,y-20,c['ref'],'P'),text(x+30,y-5,'AO3400A','N'),pin(x-60,y,'1','G',20,False),pin(x,y-40,'3','D',0,False),pin(x,y+40,'2','S',0,False)]
    internal += [line([(x-40,y),(x-25,y)]),line([(x-25,y-20),(x-25,y+20)]),line([(x-15,y-20),(x-15,y+20)]),line([(x,y-40),(x,y-20),(x-15,y-20)]),line([(x-15,y+20),(x,y+20),(x,y+40)]),line([(x-6,y+10),(x-15,y+15),(x-6,y+20)])]
    add_symbol(c,x,y,internal)
    a,b=discrete(gate,left+230,qy);wire([(left+30,qy),a]);label(left+30,qy,f'LED{i}_PWM')
    wire([b,(qx-60,qy)]);label(left+300,qy,f'LED{i}_GATE')
    wire([(qx,qy+40),(qx,qy+70)]);label(qx,qy+70,'GND')
    a,b=discrete(pull,left+230,1150);wire([(left+30,1150),a]);label(left+30,1150,f'LED{i}_GATE');wire([b,(left+310,1150)]);label(left+310,1150,'GND')
sh += [text(60,1230,'LED1 GPIO10 / LED2 GPIO0 / LED3 GPIO1. Each channel has its own MOSFET, 220R gate resistor and 10k pulldown.',size=10),text(60,1255,'GPIO4 SDA / GPIO5 SCL / GPIO6 radar RX / GPIO7 radar TX / GPIO3 presence.',size=10),text(60,1280,'Disconnect USB-A before USB-C programming until power isolation is verified.',size=10)]

sheet=dict(head=dict(docType='1',editorVersion='6.5.51',newgId=True,c_para={'Prefix Start':'1'},c_spiceCmd='null',hasIdFlag=True,uuid=uid('sheet'),x='0',y='0',importFlag=0),canvas='CA~1950~1350~#FFFFFF~yes~#CCCCCC~5~1950~1350~line~5~pixel~5~0~0',shape=sh,BBox=dict(x=40,y=20,width=1810,height=1300),colors={})
project=dict(editorVersion='6.5.51',docType='5',title='Nightlight Rev K',description='',colors={},schematics=[dict(docType='1',title='Nightlight schematic',description='',dataStr=sheet)])
(ROOT/'Nightlight-Schematic.json').write_text(json.dumps(project,indent=2))

# PCB geometry in millimetres. Native serialization converts to 10-mil units.
pads=[]; fps={}
def footprint(ref,x,y,w,h):
    c=components[ref]; fps[ref]=dict(x=x,y=y,w=w,h=h,side=c['side'],pads=[])
def pad(ref,num,x,y,w,h,drill=0,shape='RECT'):
    c=components[ref]; p=next(p for p in c['pins'] if p['number']==str(num)); obj=dict(ref=ref,num=str(num),x=x,y=y,w=w,h=h,drill=drill,shape=shape,layer=11 if drill else c['side'],net=p['net'] or '',name=p['name'])
    pads.append(obj); fps[ref]['pads'].append(obj)
# All modules and discretes mount on the front. J1 mounts on the rear.
# Radar/GY-302 use formed leads from module holes to exposed front lands.
BOARD=52
footprint('U1',24.4,18,18,22.4)
for i in range(8):
    pad('U1',i+1,41.4,19.6+i*2.54,3.6,1.5)
    pad('U1',i+9,25.4,19.6+i*2.54,3.6,1.5)
footprint('U2',.4,4.5,22,16)
for i in range(5):pad('U2',i+1,6.32+i*2.54,2.8,1.8,1.8)
footprint('U3',.4,24,18.7,14.2)
# GY-302 rotated 180 degrees: right header, VCC at top, ADDR at bottom.
for i in range(5):pad('U3',i+1,20.5,26.2+i*2.54,1.8,1.5)
USBX,USBY=23,23
footprint('J1',USBX-6,USBY-2.25,12,4.5)
for n,x in [(1,USBX-3.6),(4,USBX+3.6)]:pad('J1',n,x,USBY,1.8,2.6)
for n,x in [(5,USBX-7),(6,USBX+7)]:pad('J1',n,x,USBY,3,3)
for i,x,y in [(1,27,7.5),(2,33,7.5),(3,39,7.5)]:
    ref='Q'+str(i);footprint(ref,x-.5,y-1.5,3,3)
    pad(ref,1,x,y-1,.95,.6);pad(ref,2,x,y+1,.95,.6);pad(ref,3,x+2,y,.95,.6)
positions={'R1':(11.5,41.5,False),'R2':(23,41.5,False),'R9':(34.5,41.5,False),
 'R3':(27,10.8,False),'R4':(27,13.3,False),
 'R7':(33,10.8,False),'R8':(33,13.3,False),
 'R10':(39,10.8,False),'R11':(39,13.3,False),
 'C1':(22.5,38.2,True),'C2':(22.5,34.3,True),
 'C3':(20,2,False),'C4':(24,2,False),'C5':(21,22.8,False)}
for ref,(x,y,vertical) in positions.items():
    if vertical:
        footprint(ref,x-.625,y-1,1.25,2);pad(ref,1,x,y-1.15,1.5,1.3);pad(ref,2,x,y+1.15,1.5,1.3)
    else:
        footprint(ref,x-1,y-.625,2,1.25);pad(ref,1,x-1.15,y,1.3,1.5);pad(ref,2,x+1.15,y,1.3,1.5)
for ref,x in [('D1',9.9),('D2',21.4),('D3',32.9)]:
    footprint(ref,x,42.5,3.2,2.47);pad(ref,1,x+.45,43.735,1.3,1.9);pad(ref,2,x+2.3,43.735,1.9,1.9)
holes=[(3,3,2.2),(43,3,2.2),(3,43,2.2),(43,43,2.2)]
rf_keepouts=[(0,8.5,23.4,20.9),(27.4,33.4,39.4,40.4)]
data_pin_keepout=(USBX-2,USBY-1.7,USBX+2,USBY+1.7)
# Trimmed USB tails touch only the rear. Front SMD pads are separated by FR4;
# through vias are excluded because they also carry rear copper.
keepouts=rf_keepouts
layer_keepouts=[dict(layer=2,rect=data_pin_keepout,purpose='trimmed USB data tails')]
shell_contact_bands=[(USBX-6.35,USBY-2.6,USBX+6.35,USBY-1.9),
                     (USBX-6.35,USBY+1.9,USBX+6.35,USBY+2.6),
                     (USBX-6.35,USBY-2.6,USBX-5.65,USBY+2.6),
                     (USBX+5.65,USBY-2.6,USBX+6.35,USBY+2.6)]
module_holes={'U2':[dict(num=str(i+1),x=6.32+i*2.54,y=6.0) for i in range(5)],
              'U3':[dict(num=str(i+1),x=18.1,y=26.2+i*2.54) for i in range(5)]}
# A 3 mm perimeter expansion isolates corner hardware from module envelopes.
OFFSET=3
for f in fps.values():
    f['x']+=OFFSET;f['y']+=OFFSET
for p in pads:
    p['x']+=OFFSET;p['y']+=OFFSET
for row in module_holes.values():
    for p in row:p['x']+=OFFSET;p['y']+=OFFSET
rf_keepouts=[tuple(v+OFFSET for v in r) for r in rf_keepouts]
keepouts=rf_keepouts
data_pin_keepout=tuple(v+OFFSET for v in data_pin_keepout)
for r in layer_keepouts:r['rect']=tuple(v+OFFSET for v in r['rect'])
shell_contact_bands=[tuple(v+OFFSET for v in r) for r in shell_contact_bands]
holes=[(3,3,2.2),(49,3,2.2),(3,49,2.2),(49,49,2.2)]
model=dict(board=[BOARD,BOARD],module_holes=module_holes,rf_keepouts=rf_keepouts,data_pin_keepout=data_pin_keepout,layer_keepouts=layer_keepouts,shell_contact_bands=shell_contact_bands,components=components,footprints=fps,pads=pads,holes=holes,keepouts=keepouts)
(ROOT/'design-model.json').write_text(json.dumps(model,indent=2))
print('Native schematic: embedded symbols generated. PCB model: 24 footprints,',len(pads),'pads.')

# Stroke paths make native PCB labels visible without a font-conversion service.
GLYPHS={
 'A':['06 20 46','14 34'], 'B':['00 06 36 45 44 33 03','00 30 41 42 33'],
 'C':['40 10 01 05 16 46'], 'D':['00 06 26 45 41 20 00'],
 'E':['40 00 06 46','03 33'], 'F':['40 00 06','03 33'],
 'G':['40 10 01 05 16 46 43 23'], 'H':['00 06','40 46','03 43'],
 'I':['00 40','20 26','06 46'], 'J':['00 40 45 36 16 05'],
 'K':['00 06','40 03 46'], 'L':['00 06 46'], 'M':['06 00 23 40 46'],
 'N':['06 00 46 40'], 'O':['10 30 41 45 36 16 05 01 10'],
 'P':['06 00 30 41 42 33 03'], 'Q':['10 30 41 45 36 16 05 01 10','24 46'],
 'R':['06 00 30 41 42 33 03','23 46'], 'S':['40 10 01 02 13 33 44 45 36 06'],
 'T':['00 40','20 26'], 'U':['00 05 16 36 45 40'], 'V':['00 26 40'],
 'W':['00 16 23 36 40'], 'X':['00 46','40 06'], 'Y':['00 23 40','23 26'],
 'Z':['00 40 06 46'], '0':['10 30 41 45 36 16 05 01 10'],
 '1':['11 20 26','06 46'], '2':['00 30 41 42 06 46'],
 '3':['00 30 41 42 23 43 45 36 06'], '4':['00 03 43','40 46'],
 '5':['40 00 03 33 44 45 36 06'], '6':['40 10 01 05 16 36 45 44 33 03'],
 '7':['00 40 16'], '8':['10 30 41 42 33 13 02 01 10','13 04 05 16 36 45 44 33'],
 '9':['43 13 02 01 10 30 41 45 36 06'], '-':['03 43'],
 '+':['03 43','20 26'], '.':['26 25'], '/':['06 40'], '_':['06 46'],
 '?':['00 30 41 42 23 24','26 25']}
def strokepath(x,y,s,height):
    out=[];scale=mm(height)/6
    for i,ch in enumerate(s.upper()):
        if ch==' ':continue
        for stroke in GLYPHS.get(ch,GLYPHS['?']):
            pts=[(mm(x)+(i*6+int(pair[0]))*scale,mm(y)+(int(pair[1])-6)*scale) for pair in stroke.split()]
            out.append('M '+' L '.join(fmt(a)+' '+fmt(b) for a,b in pts))
    return ' '.join(out)
def pcbtext(x,y,s,layer=3,kind='L',height=1):
    return fields('TEXT',kind,mm(x),mm(y),mm(.15),0,'none',layer,'',mm(height),s,strokepath(x,y,s,height),'none' if kind=='N' else '',gid(),'default',0)
def track(points,width=.15,layer=3,net=''):
    return fields('TRACK',mm(width),layer,net,pointstr([(mm(x),mm(y)) for x,y in points]),gid(),0)
def pcbpad(p):
    x,y,w,h=map(mm,[p['x'],p['y'],p['w'],p['h']]); pts=pointstr([(x-w/2,y-h/2),(x+w/2,y-h/2),(x+w/2,y+h/2),(x-w/2,y+h/2)]) if p['shape']=='RECT' else ''
    return fields('PAD',p['shape'],x,y,w,h,p['layer'],p['net'],p['num'],mm(p['drill']/2),pts,0,gid(),0,'','Y',0,0,mm(.05),'')
pcbsh=[track([(0,0),(BOARD,0),(BOARD,BOARD),(0,BOARD),(0,0)],.15,10)]
for ref,fp in fps.items():
    c=components[ref]; x,y,w,h=fp['x'],fp['y'],fp['w'],fp['h']; layer=3 if c['side']==1 else 4
    internal=[pcbtext(x,y-.35,ref,layer,'P',.8),pcbtext(x,y+h/2,c['value'],layer,'N',.7),track([(x,y),(x+w,y),(x+w,y+h),(x,y+h),(x,y)],.12,layer)]
    internal += [pcbpad(p) for p in fp['pads']]
    if ref=='U1':
        internal += [pcbtext(34,20.5,'USB-C',12,'L',1),pcbtext(32,40,'ANTENNA',12,'L',.8)]
    if ref in ['U2','U3']:
        for p in fp['pads']:
            internal.append(pcbtext(p['x']-1 if ref=='U2' else p['x']+1.3,p['y']+2 if ref=='U2' else p['y']+.3,p['name'],12,'L',.7))
    attr='package`'+c['package']+'`Value`'+c['value']+'`'
    pcbsh.append(fields('LIB',mm(x+w/2),mm(y+h/2),attr,0,'',c['id'],c['side'],c['puuid'],0,0,'','yes','','')+'#@$'+'#@$'.join(internal))
    libdoc=dict(head=dict(docType='4',editorVersion='6.5.51',x=mm(x+w/2),y=mm(y+h/2),c_para=dict(package=c['package'],pre=ref[0]+'?'),uuid=c['puuid']),canvas='CA~1000~1000~#000000~yes~#FFFFFF~1~1000~1000~line~0.1~mm~1~45~~0.1~0~0',shape=internal)
    (LIB/(ref+'-footprint.json')).write_text(json.dumps(libdoc,indent=2))
for x,y,diam in holes:
    # Explicit non-plated mechanical pads avoid ambiguous HOLE conversion in Pro.
    pcbsh += [fields('PAD','ELLIPSE',mm(x),mm(y),mm(diam),mm(diam),11,'','',mm(diam/2),'',0,gid(),0,'','N',0,0,0,''),fields('CIRCLE',mm(x),mm(y),mm(2.2),mm(.12),12,gid(),0)]
for x1,y1,x2,y2 in keepouts:
    points=[(x1,y1),(x2,y1),(x2,y2),(x1,y2),(x1,y1)]
    pcbsh.append(track(points,.15,12)); pcbsh.append(pcbtext(x1+.4,y1+1.6,'DATA PIN CLEAR' if (x1,y1,x2,y2)==data_pin_keepout else 'RF KEEP CLEAR',12,'L',.8))
    # Copper cutouts are native objects on both layers; routing also avoids them.
    path='M '+' L '.join(f'{fmt(mm(x))} {fmt(mm(y))}' for x,y in points)+' Z'
    for layer in [1,2]: pcbsh.append(fields('SOLIDREGION',layer,'',path,'cutout',gid(),'',0))
for reservation in layer_keepouts:
    x1,y1,x2,y2=reservation['rect']
    points=[(x1,y1),(x2,y1),(x2,y2),(x1,y2),(x1,y1)]
    path='M '+' L '.join(f'{fmt(mm(x))} {fmt(mm(y))}' for x,y in points)+' Z'
    pcbsh.append(fields('SOLIDREGION',reservation['layer'],'',path,'cutout',gid(),'',0))
    pcbsh.append(track(points,.1,12))
pcbsh += [pcbtext(13,48.5,'NIGHTLIGHT REV K',3,'L',.9),pcbtext(32,21,'DRAFT',3,'L',.9)]
for x1,y1,x2,y2 in shell_contact_bands:
    points=[(x1,y1),(x2,y1),(x2,y2),(x1,y2),(x1,y1)]
    pcbsh.append(track(points,.1,12))
    path='M '+' L '.join(f'{fmt(mm(x))} {fmt(mm(y))}' for x,y in points)+' Z'
    pcbsh.append(fields('SOLIDREGION',2,'',path,'cutout',gid(),'',0))

routes_path=ROOT/'routes.json'
if routes_path.exists():
    routes=json.loads(routes_path.read_text())
    for r in routes['tracks']: pcbsh.append(track(r['points'],r['width'],r['layer'],r['net']))
    for v in routes['vias']: pcbsh.append(fields('VIA',mm(v['x']),mm(v['y']),mm(v['diameter']),v['net'],mm(v['drill']/2),gid(),0))
else: routes=dict(tracks=[],vias=[],unrouted='not routed')
sample=json.loads((ROOT.parent/'format-reference/pcb.txt').read_text(encoding='utf-8'))
sample['layers']=[layer for layer in sample['layers'] if not layer.split('~')[1].startswith('Inner')]
pcb=dict(head=dict(docType='3',editorVersion='6.5.51',newgId=True,c_para={},x=0,y=0,hasIdFlag=True),canvas='CA~1000~1000~#000000~yes~#FFFFFF~1~1000~1000~line~0.1~mm~1~45~~0.1~0~0~1~yes',shape=pcbsh,layers=sample['layers'],objects=sample['objects'],BBox=dict(x=0,y=0,width=mm(BOARD),height=mm(BOARD)),preference=dict(hideFootprints='',hideNets=''),DRCRULE=dict(Default=dict(trackWidth=mm(.25),clearance=mm(.2),viaHoleDiameter=mm(.7),viaHoleD=mm(.3)),isRealtime=True,isDrcOnRoutingOrPlaceVia=True,checkObjectToCopperarea=True),routerRule=dict(unit='mm',trackWidth=.25,trackClearance=.2,viaHoleD=.3,viaDiameter=.7,routerLayers=[1,2],smdClearance=.2,specialNets=[],nets=sorted({p['net'] for p in pads if p['net']}),padsCount=len(pads),skipNets=[],realtime=True),netColors={})
(ROOT/'Nightlight-PCB.json').write_text(json.dumps(pcb,indent=2))
with zipfile.ZipFile(ROOT/'Nightlight-RevK-JLCEDA-Standard.zip','w',zipfile.ZIP_DEFLATED) as z:
    for f in ['Nightlight-Schematic.json','Nightlight-PCB.json']: z.write(ROOT/f,f)
with (ROOT/'BOM.csv').open('w',newline='') as f:
    wr=csv.writer(f);wr.writerow(['Reference','Value','Footprint','Side','Status'])
    for ref,c in components.items(): wr.writerow([ref,c['value'],c['package'],'Top' if c['side']==1 else 'Bottom','Provisional' if 'PROVISIONAL' in c['package'] else ('DNP' if 'DNP' in c['value'] else 'Fit')])
print('Native documents and JLCEDA Standard import ZIP written.')
