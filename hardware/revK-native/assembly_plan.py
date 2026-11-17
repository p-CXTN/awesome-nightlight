"""Draw the component-face assembly layout from the PCB model."""
from pathlib import Path
import json
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parent
m=json.loads((ROOT/'design-model.json').read_text())
S=14;OX=55;OY=110
im=Image.new('RGB',(850,950),'#f6f8fb');d=ImageDraw.Draw(im)
def load_font(size, bold=False):
    for name in (['arialbd.ttf', 'DejaVuSans-Bold.ttf'] if bold else ['arial.ttf', 'DejaVuSans.ttf']):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default(size=size)
font=lambda n:load_font(n)
bold=lambda n:load_font(n, True)
pt=lambda x,y:(OX+x*S,OY+y*S)
rect=lambda x,y,w,h:(*pt(x,y),*pt(x+w,y+h))
d.text((55,25),'Rev K — front assembly',font=bold(30),fill='#142b43')
d.text((55,67),'52 × 52 mm carrier · USB-C faces the upper edge',font=font(20),fill='#526576')
d.rectangle(rect(0,0,*m['board']),fill='#26705a',outline='#133f32',width=2)
for x,y,diam in m['holes']:
    d.ellipse(rect(x-diam/2,y-diam/2,diam,diam),fill='#f6f8fb')
for ref,f in m['footprints'].items():
    if ref=='J1':continue
    col={'U1':'#334d74','U2':'#3b5c90','U3':'#61a4ad'}.get(ref,'#6d655c')
    if ref.startswith('D'):col='#f8d978'
    d.rectangle(rect(f['x'],f['y'],f['w'],f['h']),fill=col,outline='#172d3c',width=1)
    if ref.startswith('U'):
        name={'U1':'U1  ESP32-C3','U2':'U2  LD2410C','U3':'U3  GY-302'}[ref]
        d.text(pt(f['x']+1,f['y']+f['h']/2),name,font=bold(19),fill='white')
    else:
        d.text(pt(f['x'],f['y']-1),ref,font=font(12),fill='white')
    for p in f['pads']:
        d.rectangle(rect(p['x']-p['w']/2,p['y']-p['h']/2,p['w'],p['h']),fill='#e6c776',outline='#8c763f')
for ref,holes in m['module_holes'].items():
    for h,p in zip(holes,m['footprints'][ref]['pads']):
        d.line([pt(h['x'],h['y']),pt(p['x'],p['y'])],fill='#e8e9ed',width=4)
        x,y=pt(h['x'],h['y']);d.ellipse((x-3,y-3,x+3,y+3),fill='#b5b8bf')
f=m['footprints']['U1']
d.rectangle(rect(f['x']+(f['w']-9)/2,f['y']-1.3,9,4.3),fill='#bec3cd',outline='#172d3c',width=2)
d.text(pt(f['x']+5,f['y']+.4),'USB-C',font=bold(15),fill='#172d3c')
d.text((55,870),'Gold: solder lands · Silver links: formed sensor leads',font=font(20),fill='#526576')
d.text((55,903),'USB-A mounts on the rear; all modules and discretes mount here.',font=font(19),fill='#526576')
im.save(ROOT/'assembly-plan.png')
