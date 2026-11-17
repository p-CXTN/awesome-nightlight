const playwright = require(process.env.PLAYWRIGHT_MODULE || 'playwright');
const {chromium} = playwright;
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const pageSource = name => fs.readFileSync(path.join(__dirname,'../NightlightRevK',name),'utf8').split('R"HTML(')[1].split(')HTML"')[0].replace('{{token}}','test-token');
(async () => {
 fs.mkdirSync(path.join(__dirname,'../../.nightlight-build'), {recursive:true});
 const browser = await chromium.launch({channel:process.env.BROWSER_CHANNEL || (process.platform === 'win32' ? 'msedge' : undefined),headless:true});
 try {
  const page = await browser.newPage({viewport:{width:1100,height:850}});
  const errors=[]; page.on('pageerror',error => errors.push(error.message));
  let wifiState='idle', posted=null, offline=false, scanMode='ready', scanPolls=0, hotspot=true, wifiOffline=false;
  const testSsid = 'Home <b> & \"WiFi\"';
  const radar={busy:false,live:true,target:1,frames:20,bytes:100,presence:true,movingCm:180,movingEnergy:60,stationaryCm:0,stationaryEnergy:0,movingGates:[2,3,60],stationaryGates:[0,0,15],message:'Settings read.',error:false,settings:{maxGate:8,movingGate:6,stationaryGate:6,absence:5,moving:Array(9).fill(40),stationary:Array(9).fill(30)}};
  await page.route('http://nightlight.test/**', async route => {
   const url=new URL(route.request().url());
   if ((wifiOffline && url.pathname==='/wifi-status') || (offline && ['/status','/radar-live'].includes(url.pathname))) { await route.abort(); return; }
   let body='',contentType='text/plain',status=200;
   if (url.pathname==='/wifi-scan') {
    assert.match(route.request().postData(), /token=test-token/);
    contentType='application/json';
    if (route.request().postData().includes('refresh=1')) scanPolls=0;
    if (scanMode==='error') {status=503;body='Could not scan Wi-Fi. Select Refresh networks to retry.';}
    else if (scanPolls++===0) {status=202;body=JSON.stringify({state:'scanning'});}
    else body=JSON.stringify({state:'ready',networks:scanMode==='empty'?[]:[{ssid:'My home',rssi:-40,supported:true},{ssid:'My home',rssi:-70,supported:true},{ssid:testSsid,rssi:-55,supported:true},{ssid:'Guest open',rssi:-30,supported:false}]});
   } else if (route.request().method()==='POST') {
    posted={path:url.pathname,data:route.request().postData()};
    if(url.pathname==='/wifi'){status=202;wifiState='connecting';body='Checking Wi-Fi.';}
    else if(url.pathname==='/radar-settings'){radar.settings=savedRadarSettings;status=202;body='Reading radar settings.';}
    else body='Settings saved.';
   } else if(url.pathname==='/wifi-status') {contentType='application/json';body=JSON.stringify({state:wifiState,url:'http://192.168.1.50/',hostname:'http://nightlight-123456.local/',hotspot});}
   else if(url.pathname==='/wifi'){contentType='text/html';body=pageSource('WifiPage.h');}
   else if(url.pathname==='/status'){contentType='application/json';body=JSON.stringify({radar,lux:5.2,fresh:true,light:'Ready',darknessState:'below',leds:[0,0,0],settings:{brightness:25,darkness:10,holdSeconds:30}});}
   else if(url.pathname==='/radar-live'){contentType='application/json';body=JSON.stringify(radar);}
   else {contentType='text/html';body=pageSource('Dashboard.h');}
   await route.fulfill({status,contentType,body});
  });
  const savedRadarSettings = radar.settings; radar.settings = null;
  await page.goto('http://nightlight.test/');
  await page.waitForFunction(()=>document.querySelector('#lux').textContent.includes('5.2'));
  assert(await page.locator('#panel-settings').isVisible());
  assert(await page.locator('#settings').isVisible());
  await page.locator('#tab-settings').click();
  assert(await page.locator('#settings').isVisible());
  assert(!await page.locator('[data-test="0"]').isVisible());
  assert(await page.locator('#radar-range').isVisible());
  await page.waitForFunction(()=>!document.querySelector('#radar-range-fields').disabled);
  assert.equal(posted.path,'/radar-settings');
  assert(!await page.locator('#bands-caption').isVisible());
  await page.getByText('How do distance bands work?',{exact:true}).click();
  assert(await page.locator('#bands-caption').isVisible());
  await page.getByText('How do distance bands work?',{exact:true}).click();
  assert.equal(await page.locator('[data-band]').count(),9);
  await page.locator('#band-2').click();
  await page.waitForFunction(()=>document.querySelector('#tuning-moving-value').textContent==='60 / 100');
  await page.locator('#moving-sensitivity').fill('45');
  assert.equal(await page.locator('#tuning-moving-limit').evaluate(el=>el.style.left),'45%');
  const savedScroll = await page.evaluate(()=>scrollY);
  const testingTab = await page.locator('#tab-testing').boundingBox();
  await page.mouse.click(testingTab.x + testingTab.width/2, testingTab.y + testingTab.height/2);
  await page.waitForTimeout(50);
  const settingsTab = await page.locator('#tab-settings').boundingBox();
  await page.mouse.click(settingsTab.x + settingsTab.width/2, settingsTab.y + settingsTab.height/2);
  await page.waitForTimeout(50);
  assert(Math.abs((await page.evaluate(()=>scrollY))-savedScroll)<5, JSON.stringify({before:savedScroll,after:await page.evaluate(()=>scrollY),memory:await page.evaluate(()=>tabScroll)}));
  assert((await page.locator('nav').boundingBox()).y>=-1);
  await page.locator('#band-4').click();
  await page.locator('#moving-sensitivity').fill('55');
  assert.equal(await page.locator('#moving-value').textContent(),'55');
  await page.locator('#band-3').click(); await page.locator('#band-4').click();
  assert.equal(await page.locator('#moving-sensitivity').inputValue(),'55');
  assert.match(await page.locator('#band-draft').textContent(),/Unsaved/);
  await page.locator('#moving-gate').fill('7');
  await page.locator('#radar-read').click();
  await page.waitForTimeout(1200);
  assert.equal(await page.locator('#moving-gate').inputValue(),'7');
  await page.getByRole('button',{name:'Save this band',exact:true}).click();
  await page.waitForTimeout(200);
  assert.match(posted.data,/gate=4/); assert.match(posted.data,/moving=55/);
  await page.locator('#band-4').focus(); await page.keyboard.press('ArrowRight');
  assert.equal(await page.locator('#radar-gate-select').inputValue(),'5');
  await page.locator('#brightness').fill('33');
  await page.getByRole('button',{name:'Save settings',exact:true}).click();
  await page.waitForFunction(()=>document.querySelector('#settings-message').textContent==='Settings saved.');
  assert.match(posted.data,/brightness=33/);
  await page.locator('#tab-testing').click();
  await page.locator('#lux-open').click();
  assert.match(await page.locator('#lux-check').textContent(), /Uncovered: 5.2/);
  await page.locator('#lux-covered').click();
  assert.match(await page.locator('#lux-check').textContent(), /No drop measured/);
  await page.locator('[data-level="100"]').click();
  await page.locator('[data-test="2"]').click();
  await page.waitForTimeout(150);
  assert.match(posted.data,/brightness=100/);
  assert.equal(await page.locator('[data-test="0"]').textContent(), 'Run sequentially');
  await page.locator('[data-level="10"]').click();
  await page.locator('[data-test="4"]').click();
  await page.waitForTimeout(150);
  assert.match(posted.data,/channel=4/);
  assert.match(posted.data,/brightness=10(?:&|$)/);
  assert.equal(await page.locator('[data-test="4"]').textContent(), 'All on');
  await page.locator('[data-test="-1"]').click();
  await page.waitForTimeout(150);
  assert.match(posted.data,/channel=-1/);
  await page.locator('#tab-settings').click();
  await page.locator('#tab-settings').focus();await page.keyboard.press('ArrowLeft');
  assert(await page.locator('#panel-firmware').isVisible());
  for (const width of [1100,390,320]) {
   await page.setViewportSize({width,height:850});
   for(const tab of ['settings','testing','wifi','firmware']) {
    await page.locator('#tab-'+tab).click();
    assert(await page.evaluate(()=>{const widths=[...document.querySelectorAll('nav[role=tablist] button')].map(b=>b.getBoundingClientRect().width);return Math.max(...widths)-Math.min(...widths)<2;}));
    assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),`overflow at ${width} in ${tab}`);
    await page.screenshot({path:path.join(__dirname,`../../.nightlight-build/ui-${tab}-${width}.png`),fullPage:true});
   }
  }
  await page.locator('#tab-settings').click();
  await page.screenshot({path:path.join(__dirname,'../../.nightlight-build/settings-mobile.png'),fullPage:true});
  await page.getByText('How do distance bands work?',{exact:true}).click();
  await page.locator('figure:has(#bands-caption)').screenshot({path:path.join(__dirname,'../../.nightlight-build/radar-figure.png')});
  await page.getByText('How do distance bands work?',{exact:true}).click();
  await page.setViewportSize({width:1100,height:850});
  await page.screenshot({path:path.join(__dirname,'../../.nightlight-build/settings-desktop.png'),fullPage:true});
  offline=true;
  await page.waitForFunction(()=>document.querySelector('#lux').textContent==='Offline');
  await page.locator('#tab-testing').click(); await page.locator('#lux-open').click();
  assert.match(await page.locator('#lux-check').textContent(),/fresh/);
  offline=false;
  await page.waitForFunction(()=>document.querySelector('#lux').textContent.includes('5.2'));
  await page.locator('#tab-wifi').click();
  assert(!await page.locator('#static-fields').isVisible());
  await page.waitForFunction(()=>document.querySelector('#ssid').options.length===4 && !document.querySelector('#ssid').disabled);
  assert(await page.locator('#ssid option[value="Guest open"]').isDisabled());
  await page.locator('#ssid').selectOption(testSsid);
  assert.equal(await page.locator('#ssid').inputValue(),testSsid);
  assert.equal(await page.locator('#ssid b').count(),0);
  await page.locator('#ssid').selectOption('My home');
  await page.locator('#password').fill('test-password');
  await page.locator('#scan-networks').click();
  await page.waitForFunction(()=>!document.querySelector('#scan-networks').disabled);
  assert.equal(await page.locator('#ssid').inputValue(),'My home');
  assert.equal(await page.locator('#password').inputValue(),'test-password');
  scanMode='empty';await page.locator('#scan-networks').click();
  await page.waitForFunction(()=>document.querySelector('#scan-message').textContent.includes('No supported'));
  assert(await page.locator('#save').isDisabled());
  scanMode='error';await page.locator('#scan-networks').click();
  await page.waitForFunction(()=>document.querySelector('#scan-message').textContent.includes('Could not scan'));
  assert(!await page.locator('#scan-networks').isDisabled());
  scanMode='ready';await page.locator('#scan-networks').click();
  await page.waitForFunction(()=>document.querySelector('#ssid').options.length===4 && !document.querySelector('#ssid').disabled);
  await page.locator('#ssid').selectOption('My home');await page.locator('#password').fill('test-password');
  await page.setViewportSize({width:390,height:850});
  await page.screenshot({path:path.join(__dirname,'../../.nightlight-build/wifi-selector-mobile.png'),fullPage:true});
  assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
  await page.locator('#addressing').selectOption('static');
  for(const [id,value] of Object.entries({ip:'192.168.1.50',gateway:'192.168.1.1',dns:'192.168.1.1'}))await page.locator('#'+id).fill(value);
  await page.locator('#save').click();
  await page.waitForFunction(()=>document.querySelector('#message').textContent.includes('Checking'));
  assert.match(posted.data,/token=test-token/);assert.match(posted.data,/addressing=static/);assert.match(posted.data,/ssid=My\+home/);
  wifiState='failed';
  await page.waitForFunction(()=>document.querySelector('#message').textContent.includes('Could not connect'));
  assert(!await page.locator('#save').isDisabled());
  await page.locator('#addressing').selectOption('dhcp');
  await page.locator('#save').click();
  await page.waitForFunction(()=>document.querySelector('#message').textContent.includes('Checking'));
  assert(!posted.data.includes('&ip='));
  wifiState='saved';
  await page.waitForFunction(()=>!document.querySelector('#result').hidden);
  assert.equal(await page.locator('#dashboard').getAttribute('href'),'http://192.168.1.50/');
  assert.equal(await page.locator('#password').inputValue(),'');
  assert(await page.locator('#save').isDisabled());
  assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
  wifiState='idle';hotspot=false;
  await page.goto('http://nightlight.test/dashboard#wifi');

  await page.waitForFunction(()=>document.querySelector('#ssid').options.length===4 && !document.querySelector('#ssid').disabled);
  assert(!await page.locator('#fields').isDisabled());
  assert.equal(await page.locator('#access').count(),0);
  await page.locator('#ssid').selectOption('My home');
  await page.locator('#password').fill('test-password');
  await page.locator('#save').click();
  await page.waitForFunction(()=>document.querySelector('#message').textContent.includes('Checking'));
  assert.equal(posted.path,'/wifi');
  assert(await page.locator('#wifi-handoff').isVisible());
  assert.match(await page.locator('#handoff-message').textContent(), /My home/);
  assert.equal(await page.locator('#handoff-link').getAttribute('href'),'http://nightlight-123456.local/');
  wifiOffline=true;
  await page.waitForFunction(()=>document.querySelector('#message').textContent.includes('Follow the connection'));
  assert(await page.locator('#wifi-handoff').isVisible());
  wifiOffline=false;wifiState='failed';
  await page.waitForFunction(()=>document.querySelector('#message').textContent.includes('Could not connect'));
  assert(!await page.locator('#save').isDisabled());
  assert(!await page.locator('#wifi-handoff').isVisible());
  await page.locator('#save').click();
  await page.waitForFunction(()=>document.querySelector('#message').textContent.includes('Checking'));
  wifiState='saved';
  await page.waitForFunction(()=>!document.querySelector('#result').hidden);
  wifiState='idle';hotspot=true;
  await page.goto('http://nightlight.test/wifi');
  await page.waitForFunction(()=>document.querySelector('#ssid').options.length===4 && !document.querySelector('#ssid').disabled);
  await page.locator('#ssid').selectOption('My home');
  assert(!await page.locator('#save').isDisabled());
  assert.deepEqual(errors,[]);
  console.log('PASS: tabs, keyboard navigation, lamp save, mobile layouts, Wi-Fi scanning/selection/refresh/errors, home-network switching and interrupted handoff, static/DHCP payloads, failed-join retry, restart handoff, no browser errors.');
 } finally {await browser.close();}
})().catch(error=>{console.error(error);process.exitCode=1;});
