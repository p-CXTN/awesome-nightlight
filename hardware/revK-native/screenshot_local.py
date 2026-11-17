"""Capture local SVG drawings with all network access disabled."""
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parent
from playwright.sync_api import sync_playwright
with sync_playwright() as p:
    b=p.chromium.launch(headless=True)
    ctx=b.new_context(offline=True)
    ctx.route('http://**/*',lambda route:route.abort())
    ctx.route('https://**/*',lambda route:route.abort())
    page=ctx.new_page()
    for name,w,h in [('schematic',1950,1350),('pcb',1300,1000)]:
        page.set_viewport_size(dict(width=w,height=h))
        page.set_content('<html><body style="margin:0">'+(ROOT/(name+'.svg')).read_text(encoding='utf-8')+'</body></html>')
        page.screenshot(path=str(ROOT/(name+'.png')),timeout=10000)
    b.close()
print('Local previews captured; network disabled.')
