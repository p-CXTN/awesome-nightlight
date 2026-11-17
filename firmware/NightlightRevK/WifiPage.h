#pragma once

constexpr char wifiPage[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="csrf-token" content="{{token}}"><title>Nightlight Wi-Fi setup</title>
<style>
:root{color-scheme:light;--ink:#203b40;--muted:#607579;--accent:#176b65;--line:#dce7e4;--soft:#eff5f3}
*{box-sizing:border-box}html{scroll-padding-top:6rem}body{margin:0;background:#f3f6f4;color:var(--ink);font:15px/1.6 system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}
main{max-width:800px;margin:auto;padding:40px 28px 64px}h1{font-size:30px;line-height:1.2;letter-spacing:-1px;margin:0 0 24px;font-weight:750}h2{font-size:20px;line-height:1.35;letter-spacing:-.35px;margin:0 0 14px}h3{font-size:15px;margin:0 0 12px}p{margin:12px 0}a{color:var(--accent);overflow-wrap:anywhere}
.live-bar{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.live-bar>span{background:#fff;border:1px solid var(--line);border-radius:16px;padding:16px 20px;font-size:12px;color:var(--muted);letter-spacing:.2px}.live-bar strong{display:block;font-size:22px;line-height:1.4;letter-spacing:-.4px;color:var(--ink);margin-top:4px}
nav[role=tablist]{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));position:sticky;top:0;z-index:20;padding:8px;gap:4px;margin:20px 0 24px;background:#e7eeeb;border-radius:14px;box-shadow:0 8px 16px #f3f6f4}nav[role=tablist] button{color:var(--muted);background:transparent;border-color:transparent;min-width:0;padding:10px 6px}nav[role=tablist] button[aria-selected=true]{background:#fff;color:var(--accent);box-shadow:0 2px 6px #173d3210}
.grid{display:grid;grid-template-columns:1fr;gap:20px;align-items:start}.wide{grid-column:1/-1}.tab-panel{outline:none;scroll-margin-top:6rem}section{min-width:0;background:#fff;border:1px solid var(--line);border-radius:20px;padding:26px;box-shadow:0 4px 20px #183b3104}.section-heading{display:flex;justify-content:space-between;align-items:center;gap:12px;margin-bottom:18px}.section-heading h2{margin:0}.section-heading button{flex-shrink:0}
form,fieldset{min-width:0}form{width:100%}fieldset{padding:0;margin:0;border:0}fieldset:disabled{opacity:.6}legend{font-size:15px;font-weight:700;padding:0;margin-bottom:8px}label{display:block;font-size:13px;font-weight:600;margin:18px 0 7px}input,button,select{font:inherit}input:not([type=checkbox]):not([type=range]),select{display:block;width:100%;min-width:0;border:1px solid #b9cbc6;border-radius:10px;background:#fbfcfb;color:var(--ink);padding:11px 12px;min-height:46px}input[type=range]{display:block;width:100%;margin:0;accent-color:var(--accent);min-height:40px}input[type=checkbox]{width:17px;height:17px;margin:0;accent-color:var(--accent)}label:has(input[type=checkbox]){display:flex;align-items:center;gap:9px;font-weight:400;margin-top:12px}input[type=file]{max-width:100%;font-size:13px}input[type=file]::file-selector-button{border:0;border-radius:7px;background:#e5eeea;color:var(--ink);font:inherit;font-weight:600;padding:9px 12px;margin-right:12px}output{float:right;color:var(--accent);font-weight:700;font-variant-numeric:tabular-nums}
button{min-height:44px;padding:10px 16px;border:1px solid transparent;border-radius:10px;background:var(--soft);color:var(--accent);font-size:13px;font-weight:650;line-height:1.4;cursor:pointer;transition:background .15s,box-shadow .15s}button:hover:not(:disabled){background:#e0ece7}button.primary{background:var(--accent);color:#fff}button.primary:hover:not(:disabled){background:#10544f}button.danger{background:#fbefeb;color:#9a4734}button.danger:hover:not(:disabled){background:#f6e0d8}button:disabled{opacity:.45;cursor:not-allowed}button:focus-visible,input:focus-visible,select:focus-visible,summary:focus-visible,a:focus-visible{outline:3px solid #d39839;outline-offset:3px}.actions{display:flex;align-items:center;gap:10px;flex-wrap:wrap;margin:20px 0 0}form>.actions,fieldset>.actions{border-top:1px solid var(--line);padding-top:18px;margin-top:24px}.section-heading button,.field-heading button{background:transparent;border:1px solid var(--line)}.field-heading{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-top:20px;margin-bottom:8px}.field-heading label{margin:0}.field-heading button{padding:8px 12px;font-size:12px}
.note,.message,.intro,figcaption{font-size:13px;color:var(--muted)}.intro{margin-top:-5px}.message{margin:12px 0 0;padding:10px 12px;background:var(--soft);border-radius:9px;overflow-wrap:anywhere}.message:empty{display:none}.error{color:#9b392d;background:#fff0eb}details{border-top:1px solid var(--line);margin:18px 0 0;padding-top:12px}summary{cursor:pointer;font-size:12px;font-weight:600;color:var(--muted)}details p{margin:10px 0;font-size:13px}details p:last-child{margin-bottom:0}.steps,ol{padding-left:20px;font-size:13px;color:var(--muted)}li{padding-left:4px;margin:8px 0}
.radar-grid{display:grid;grid-template-columns:1fr;gap:24px;margin-top:20px}#radar-gate{border-top:1px solid var(--line);padding-top:24px}.band-tabs{display:grid;grid-template-columns:repeat(9,minmax(44px,1fr));overflow-x:auto;gap:4px;padding:4px;margin:12px 0;background:var(--soft);border-radius:12px}.band-tabs button{padding:10px 0;background:transparent;color:var(--muted)}.band-tabs button[aria-selected=true]{background:var(--accent);color:white}#band-title{font-weight:700;margin:14px 0 2px}#tuning-distance{margin:0 0 12px}.slider-scale{display:flex;justify-content:space-between;gap:12px;font-size:11px;color:var(--muted)}.tuning-reading{display:flex;justify-content:space-between;gap:12px;font-size:12px;margin:18px 0 7px}.tuning-reading strong{font-variant-numeric:tabular-nums}#band-panel>.actions{margin-top:20px}
.preset-actions{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-top:10px}.preset-actions button{padding:8px 4px;font-size:12px}.test-actions{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:20px}.individual-actions{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-top:10px}.test-stop{display:flex;justify-content:flex-end;margin-top:16px;padding-top:16px;border-top:1px solid var(--line)}#lux-open,#lux-covered{flex:1 1 150px}
.reading{display:flex;justify-content:space-between;gap:20px;padding:12px 0;border-bottom:1px solid var(--line);font-size:13px}.reading:last-child{border:0}dl{margin:0}dt{color:var(--muted)}dd{margin:0;text-align:right;font-weight:600}.range-view,.gate-view{margin:24px 0 0;padding-top:20px;border-top:1px solid var(--line)}.range-row{display:grid;grid-template-columns:80px 1fr;align-items:center;gap:10px;margin:12px 0;font-size:12px}.range-track{position:relative;height:8px;margin:0 8px;background:var(--soft);border-radius:9px}.range-mark{position:absolute;top:50%;width:12px;height:12px;transform:translate(-50%,-50%);border-radius:50%;background:var(--accent)}.range-mark.stationary{border-radius:3px;background:#b87f31;transform:translate(-50%,-50%) rotate(45deg)}.range-scale{display:flex;justify-content:space-between;margin:0 8px 0 98px;font-size:11px;color:var(--muted)}.gate-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(90px,1fr));gap:14px;margin-top:16px}.gate-energy{min-width:0;font-size:12px}.energy-value{display:flex;justify-content:space-between;gap:5px}.energy-track{position:relative;height:6px;background:var(--soft);border-radius:5px;margin:0 0 10px}.energy-fill{display:block;height:100%;background:var(--accent);border-radius:5px}.energy-fill.stationary{background:#b87f31}.energy-limit{position:absolute;top:-3px;width:2px;height:12px;background:var(--ink)}svg{width:100%;height:auto}figure{max-width:540px;margin:20px auto}figcaption{font-size:12px}.band-legend{font-size:12px}
#panel-wifi,#panel-firmware{max-width:720px;margin-inline:auto}#static-fields{padding:0 16px 16px;background:var(--soft);border-radius:12px;margin-top:18px}#static-fields>.note{padding-top:14px}#wifi-handoff,#result{background:var(--soft);padding:12px 16px;margin-top:20px;border-radius:12px;font-size:13px}#panel-wifi section>p:last-child{border-top:1px solid var(--line);padding-top:20px;margin-top:24px}[hidden]{display:none!important}

@media(max-width:600px){main{padding:24px 16px 40px}h1{font-size:26px;margin-bottom:20px}.live-bar{gap:8px}.live-bar>span{padding:12px 10px;font-size:11px;border-radius:12px}.live-bar strong{font-size:17px}nav[role=tablist]{margin:16px 0 20px;padding:6px;border-radius:12px}nav[role=tablist] button{font-size:12px}.grid{gap:16px}section{padding:20px;border-radius:16px}h2{font-size:19px}.section-heading{align-items:flex-start;flex-direction:column;gap:10px}.field-heading{gap:8px}.field-heading button{font-size:11px;padding:8px}button{padding:10px 12px}.actions>button{flex-grow:1}.test-stop button{width:100%}.reading{gap:12px}.live-bar strong{overflow-wrap:anywhere}}

@media(max-width:600px){.band-tabs{grid-template-columns:repeat(5,minmax(0,1fr));overflow:visible}.band-tabs button{min-width:0}}
@media(prefers-reduced-motion:reduce){button{transition:none}}
main{max-width:760px}
</style></head><body><main>
<h1>Connect your Nightlight</h1>
<section>
<ol><li>Keep this page open while setting up your home connection.</li><li>Select your home 2.4 GHz Wi-Fi network and enter its password.</li><li>After the connection check, Nightlight saves the details and restarts. Rejoin your home Wi-Fi and open the dashboard.</li></ol>
<form id="wifi"><fieldset id="fields">
<div class="field-heading"><label for="ssid">Home Wi-Fi network</label><button id="scan-networks" type="button">Refresh networks</button></div><select id="ssid" name="ssid" required><option value="">Select a network</option></select>
<p id="scan-message" class="note" role="status" aria-live="polite"></p>
<label for="password">Home Wi-Fi password</label><input id="password" name="password" type="password" minlength="8" maxlength="63" autocomplete="new-password" required>
<label><input id="show-password" type="checkbox"> Show password</label>
<p class="note">Use a password-protected 2.4 GHz personal network. Enterprise and sign-in-page networks are not supported.</p>
<label for="addressing">IP address</label><select id="addressing" name="addressing"><option value="dhcp">Automatic (DHCP)</option><option value="static">Choose a static IP</option></select>
<fieldset id="static-fields" hidden disabled>
<p class="note">Choose an unused address outside your router's DHCP pool, or reserve it on the router. Incorrect values can make the dashboard unreachable. The home subnet must not overlap 192.168.4.0/24.</p>
<label for="ip">Nightlight IPv4 address</label><input id="ip" name="ip" placeholder="192.168.1.50" inputmode="decimal" required>
<label for="gateway">Router / gateway</label><input id="gateway" name="gateway" placeholder="192.168.1.1" inputmode="decimal" required>
<label for="subnet">Subnet mask</label><input id="subnet" name="subnet" value="255.255.255.0" inputmode="decimal" required>
<label for="dns">DNS server</label><input id="dns" name="dns" placeholder="192.168.1.1" inputmode="decimal" required>
</fieldset>
<div class="actions"><button class="primary" id="save" type="submit" disabled>Check connection and save</button></div></fieldset></form>
<p id="message" role="status" aria-live="polite"></p>
<div id="wifi-handoff" hidden><p id="handoff-message"></p><p>After joining the selected network, open <a id="handoff-link">the lamp dashboard</a>. If the link does not resolve, find Nightlight in your router's device list. If the connection fails, wait about a minute, then reopen the dashboard on the previous network.</p></div>
<div id="result" hidden><p>Nightlight will restart in 15 seconds. Rejoin your home Wi-Fi, then open:</p><p><a id="dashboard">Open dashboard</a></p><p class="note">You can also use <a id="hostname"></a> if your network supports local names.</p></div>
<p class="note">To erase saved home Wi-Fi, hold BOOT for 10 seconds while powered on, then release. Lamp and radar settings are kept. Do not hold BOOT while powering on.</p>
<a href="/dashboard">Dashboard</a>
</section></main><script>
const byId = id => document.getElementById(id);
let checking = false, saved = false, onHotspot = false, dashboardHostname = '';
let scanning = false, scanAttempted = false;
async function scanNetworks() {
  if (scanning || checking || saved) return;
  scanning = true; scanAttempted = true;
  byId('scan-networks').disabled = true;
  byId('ssid').disabled = true;
  byId('save').disabled = true;
  byId('scan-message').textContent = 'Searching for nearby Wi-Fi…';
  const previous = byId('ssid').value;
  try {
    for (let attempt = 0; attempt < 20; ++attempt) {
      const data = new URLSearchParams({token: document.querySelector('meta[name=csrf-token]').content});
      if (!attempt) data.set('refresh', '1');
      const response = await fetch('/wifi-scan', {method:'POST', body:data, signal:AbortSignal.timeout(6000)});
      if (!response.ok) throw new Error(await response.text());
      const result = await response.json();
      if (checking || saved) return;
      if (result.state === 'scanning') {
        await new Promise(resolve => setTimeout(resolve, 750));
        continue;
      }
      const networks = new Map();
      for (const network of result.networks) {
        const existing = networks.get(network.ssid);
        if (!existing || (network.supported && !existing.supported) ||
            (network.supported === existing.supported && network.rssi > existing.rssi)) networks.set(network.ssid, network);
      }
      const select = byId('ssid');
      select.replaceChildren(new Option('Select a network', ''));
      let available = 0;
      for (const network of [...networks.values()].sort((a,b) => b.rssi - a.rssi)) {
        const signal = network.rssi >= -60 ? 'Strong' : network.rssi >= -75 ? 'Fair' : 'Weak';
        const option = new Option(network.ssid + ' — ' + (network.supported ? signal : 'Not supported'), network.ssid);
        option.disabled = !network.supported;
        select.add(option);
        if (network.supported) ++available;
      }
      if (networks.get(previous)?.supported) select.value = previous;
      else byId('password').value = '';
      byId('scan-message').textContent = available ? 'Select your Wi-Fi network and enter its password.' : 'No supported networks found. Check that your router’s 2.4 GHz Wi-Fi is on, then refresh.';
      return;
    }
    throw new Error('Scan timed out. Select Refresh networks to retry.');
  } catch (error) {
    byId('scan-message').textContent = error.message || 'Could not scan Wi-Fi. Select Refresh networks to retry.';
  } finally {
    scanning = false;
    byId('scan-networks').disabled = false;
    byId('ssid').disabled = false;
    byId('save').disabled = !byId('ssid').value;
  }
}
byId('scan-networks').onclick = scanNetworks;
byId('ssid').onchange = () => { byId('password').value = ''; byId('save').disabled = !byId('ssid').value; };

byId('show-password').onchange = event => byId('password').type = event.target.checked ? 'text' : 'password';
byId('addressing').onchange = () => {
  const manual = byId('addressing').value === 'static';
  byId('static-fields').hidden = !manual;
  byId('static-fields').disabled = !manual;
};
function message(text, error = false) {
  byId('message').textContent = text;
  byId('message').classList.toggle('error', error);
}
async function status() {
  try {
    const response = await fetch('/wifi-status', {cache:'no-store', signal:AbortSignal.timeout(4000)});
    if (!response.ok) throw new Error();
    const state = await response.json();
    onHotspot = state.hotspot;
    dashboardHostname = state.hostname;
    byId('handoff-link').href = dashboardHostname;
    byId('handoff-link').textContent = dashboardHostname;
    if (state.state === 'saved') {
      checking = false; saved = true;
      byId('fields').disabled = true;
      byId('password').value = '';
      message('Connected. Wi-Fi saved.');
      byId('result').hidden = false;
      byId('wifi-handoff').hidden = true;
      for (const [id, url] of [['dashboard',state.url],['hostname',state.hostname]]) {
        byId(id).href = url; byId(id).textContent = url;
      }
    } else if (state.state === 'connecting') {
      checking = true; byId('fields').disabled = true;
      message(onHotspot ? 'Checking connection (up to 45 seconds). Stay on the Nightlight hotspot.' : 'Checking connection (up to 45 seconds). The dashboard may disconnect while the lamp changes networks.');
    } else {
      checking = false;
      byId('fields').disabled = false;
      if (!scanAttempted) scanNetworks();
      const errors = {'failed':'Could not connect. Check the Wi-Fi name, password and signal, then retry. Your previous saved network is unchanged.', 'storage-error':'Connected, but could not save. Retry; if this continues, service the device.', 'invalid':'Could not apply the IP settings. Check the addresses and retry.'};
      if (errors[state.state]) { byId('wifi-handoff').hidden = true; message(errors[state.state], true); }
    }
  } catch {
    if (!saved) message(checking ? 'The lamp is changing networks. Follow the connection instructions below.' : 'Connection interrupted. Reconnect to the lamp’s network and reload this page.', !checking);
  }
}
byId('wifi').onsubmit = async event => {
  event.preventDefault();
  if (checking || saved || scanning) return;
  const data = new URLSearchParams(new FormData(event.target));
  data.set('token', document.querySelector('meta[name=csrf-token]').content);
  checking = true; byId('fields').disabled = true;
  byId('wifi-handoff').hidden = false;
  byId('handoff-message').textContent = onHotspot ? 'Stay on the Nightlight hotspot until the connection check finishes.' : 'The lamp is switching to “' + data.get('ssid') + '”. Connect your phone or computer to that network to reach it.';
  byId('handoff-link').href = data.get('addressing') === 'static' ? 'http://' + data.get('ip') + '/' : dashboardHostname;
  byId('handoff-link').textContent = byId('handoff-link').href;
  try {
    const response = await fetch('/wifi', {method:'POST',body:data,signal:AbortSignal.timeout(6000)});
    message(await response.text(), !response.ok);
    if (!response.ok) { checking = false; byId('fields').disabled = false; byId('wifi-handoff').hidden = true; }
  } catch { message('The connection was interrupted. Follow the instructions below to find the lamp.', true); }
  await status();
};
status();
setInterval(() => { if (checking) status(); }, 2000);
</script></body></html>)HTML";
