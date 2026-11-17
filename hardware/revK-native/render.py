"""Render native document primitives for local visual verification."""
import json,html
from pathlib import Path
ROOT=Path(__file__).resolve().parent
def esc(s):return html.escape(str(s),quote=True)
def attrs(**kw):return ' '.join(k.replace('_','-')+'="'+esc(v)+'"' for k,v in kw.items())
def el(tag,**kw):return '<'+tag+' '+attrs(**kw)+'/>'
def txt(x,y,s,color='#172b40',size=12,anchor='start',**kw):return '<text '+attrs(x=x,y=y,fill=color,font_size=size,text_anchor=anchor,**kw)+'>'+esc(s)+'</text>'
def schem(s):
    f=s.split('~');t=f[0]
    if t=='LIB':return ''.join(schem(x) for x in s.split('#@$')[1:])
    if t in ['PL','W']:return el('polyline',points=f[1],stroke=f[2],stroke_width=f[3],fill='none')
    if t=='R':return el('rect',x=f[1],y=f[2],width=f[5],height=f[6],stroke=f[7],stroke_width=f[8],fill='none')
    if t=='T' and f[13]=='1':return txt(f[2],f[3],f[12],f[5],float(f[7].replace('pt',''))*1.15,f[14])
    if t=='N':return txt(f[8],f[9],f[5],f[4],10)
    if t=='P':
        chunks=s.split('^^');p=chunks[2].split('~');out=el('path',d=p[0],stroke=p[1],fill='none',stroke_width=1)
        for k in [3,4]:
            a=chunks[k].split('~')
            if a[0]=='1':out+=txt(a[1],a[2],a[4],'#880000',9 if k==3 else 7,a[5])
        return out
    if t=='O':return el('path',d=f[4],stroke=f[5],fill='none')
    if t=='J':return el('circle',cx=f[1],cy=f[2],r=f[3],fill=f[4])
    return ''
sch=json.loads((ROOT/'Nightlight-Schematic.json').read_text())['schematics'][0]['dataStr']
svg='<svg xmlns="http://www.w3.org/2000/svg" width="1950" height="1350" viewBox="0 0 1950 1350"><rect width="1950" height="1350" fill="white"/><g font-family="Arial">'+''.join(schem(s) for s in sch['shape'])+'</g></svg>'
(ROOT/'schematic.svg').write_text(svg,encoding='utf-8')
pcb=json.loads((ROOT/'Nightlight-PCB.json').read_text())
shapes=[]
for s in pcb['shape']:
    if s.startswith('LIB~'):shapes.extend(s.split('#@$')[1:])
    else:shapes.append(s)
COL={1:'#d94841',2:'#427de2',3:'#d9eadf',4:'#d9eadf',10:'#ffffff',12:'#aaaec1'}
def pcbshape(s,side):
    f=s.split('~');t=f[0];out=''
    if t=='TRACK':
        l=int(f[2]);opacity=1 if l in [side,side+2,10] else .23
        return el('polyline',points=f[4],stroke=COL.get(l,'#777'),stroke_width=f[1],stroke_linecap='round',stroke_linejoin='round',fill='none',opacity=opacity)
    if t=='PAD':
        x,y,w,h=map(float,f[2:6]);l=int(f[6]);opacity=1 if l in [side,11] else .3
        if f[1]=='ELLIPSE':out=el('ellipse',cx=x,cy=y,rx=w/2,ry=h/2,fill='#e1b953',opacity=opacity)
        else:out=el('rect',x=x-w/2,y=y-h/2,width=w,height=h,fill='#e1b953',opacity=opacity)
        r=float(f[9]);
        if r:out+=el('circle',cx=x,cy=y,r=r,fill='#0c1723')
        return out
    if t=='VIA':return el('circle',cx=f[1],cy=f[2],r=float(f[3])/2,fill='#b7bcc6')+el('circle',cx=f[1],cy=f[2],r=f[5],fill='#0c1723')
    if t=='HOLE':return el('circle',cx=f[1],cy=f[2],r=float(f[3])/2,fill='#fff')
    if t=='TEXT':
        if f[12]=='none':return ''
        l=int(f[7])
        if l not in [side+2,12]:return ''
        if l==12 and f[10] not in ['RF KEEP CLEAR','DATA PIN CLEAR','TX','RX','OUT','GND','VCC','ADDR','SDA','SCL','USB-C','ANTENNA']:return ''
        return txt(f[2],f[3],f[10],COL.get(l,'#aaa'),float(f[9])*.9)
    if t=='SOLIDREGION' and f[4]=='cutout' and int(f[1])==side:
        return el('path',d=f[3],fill='#e47977',opacity=.10,stroke='#e47977',stroke_width=.3)
    return ''
w=52/.254;h=52/.254
out=['<svg xmlns="http://www.w3.org/2000/svg" width="1300" height="1000" viewBox="0 0 1300 1000">','<style>text{font-family:Arial,sans-serif}</style><rect width="1300" height="1000" fill="#0c1723"/>',txt(35,40,'Nightlight Rev K - routed PCB draft','#fff',25),txt(35,70,'52 x 52 mm | 2 layers | Front-mounted modules | Bottom LEDs','#c8d8e8',16)]
for side,ox in [(1,50),(2,700)]:
    out.append(txt(ox,110,'FRONT / top copper' if side==1 else 'REAR / bottom copper - front projection','#fff',16))
    out.append(f'<g transform="translate({ox},145) scale(2.7)">')
    out.append(el('rect',x=0,y=0,width=w,height=h,fill='#153d32',stroke='#fff',stroke_width=.4))
    out.extend(pcbshape(s,side) for s in shapes if s.startswith('SOLIDREGION'))
    out.extend(pcbshape(s,side) for s in shapes if s.startswith('TRACK'))
    out.extend(pcbshape(s,side) for s in shapes if s.startswith(('PAD','VIA','HOLE','TEXT')))
    out.append('</g>')
out += [txt(35,860,'Measured module outlines; USB shell legs use broad solder lands; D+/D- tails trimmed.','#d3dfeb',17),txt(35,890,'Red: top copper. Blue: bottom copper. Faint features: opposite side. Shaded regions: RF exclusions.','#d3dfeb',17),txt(35,920,'See validation.json for checks. Native JLCEDA DRC and fabrication export are required.','#d3dfeb',17),'</svg>']
(ROOT/'pcb.svg').write_text(''.join(out),encoding='utf-8')
print('Rendered native schematic and PCB primitives to SVG.')
