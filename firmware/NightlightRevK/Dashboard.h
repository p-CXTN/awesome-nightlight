#pragma once

constexpr char dashboardPage[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="csrf-token" content="{{token}}">
<title>Nightlight</title>
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
</style>
</head>
<body>
<main>
<h1>Nightlight</h1>
<div class="live-bar" aria-label="Live readings"><span>Room light <strong id="lux">Checking</strong></span><span>People <strong id="status-presence">Checking</strong></span><span>Lamp <strong id="lamp-now">Checking</strong></span></div><p id="status-message" class="message" role="status"></p>
<nav role="tablist" aria-label="Nightlight pages"><button id="tab-settings" role="tab" aria-selected="true" aria-controls="panel-settings">Settings</button><button id="tab-testing" role="tab" aria-selected="false" aria-controls="panel-testing" tabindex="-1">Testing</button><button id="tab-wifi" role="tab" aria-selected="false" aria-controls="panel-wifi" tabindex="-1">Wi-Fi</button><button id="tab-firmware" role="tab" aria-selected="false" aria-controls="panel-firmware" tabindex="-1">Update</button></nav><div id="panel-settings" class="grid tab-panel" role="tabpanel" aria-labelledby="tab-settings"><section aria-labelledby="settings-title">
<h2 id="settings-title">Light</h2><p class="intro">Turn on when the room is dark and someone is here.</p><details><summary>First setup?</summary><p>Connect Wi-Fi, check the hardware in Testing, then choose brightness and light levels here.</p></details>
<form id="settings">
<label for="brightness">Brightness <output id="brightness-value">25%</output></label>
<input type="range" id="brightness" name="brightness" min="1" max="100" required>
<label for="darkness">Turn on below (lux)</label>
<input type="number" id="darkness" name="darkness" min="0" max="1000" step="0.1" aria-describedby="darkness-help" required>
<details><summary>How do I choose this?</summary><p id="darkness-help" class="note">At the room darkness you want, read Room light. Set this just above it: 8 lux → try 10.</p></details>
<label for="shutoff">Turn off above (lux)</label>
<input type="number" id="shutoff" name="shutoff" min="0" max="1000" step="0.1" value="0" required>
<details><summary>How do I choose the off value?</summary><p class="note">Set this above the normal lamp-on reading: 25 lux → try 40. Use 0 to disable light-based turn-off.</p></details>
<input id="holdSeconds" name="holdSeconds" type="hidden" value="0">
<div class="actions"><button class="primary" type="submit">Save settings</button></div>
</form>
<p id="settings-message" class="message" role="status"></p>
</section><section class="wide" aria-labelledby="radar-settings-title"><div class="section-heading"><h2 id="radar-settings-title">People detection</h2><button id="radar-read" type="button">Reload radar settings</button></div>
<p id="radar-settings-message" class="message" role="status"></p>
<div class="radar-grid"><form id="radar-range">
<fieldset id="radar-range-fields" disabled>
<legend>Where to look</legend>
<label for="moving-gate">Farthest band for movement</label>
<input id="moving-gate" name="moving" type="number" min="2" max="8" required>
<label for="stationary-gate">Farthest band for small movements</label>
<input id="stationary-gate" name="stationary" type="number" min="2" max="8" required>
<label for="radar-absence">Wait after someone leaves (seconds)</label>
<input id="radar-absence" name="absence" type="number" min="0" max="65535" required>
<details><summary>How long should it wait?</summary><p class="note">Start with 5 seconds. Increase it if the light goes out while someone sits still.</p></details>
<div class="actions"><button class="primary" type="submit">Save range and delay</button></div>
</fieldset>
</form>
<form id="radar-gate">
<fieldset id="radar-gate-fields" disabled>
<legend>Adjust each distance band</legend>
<input id="radar-gate-select" name="gate" type="hidden" value="0"><div class="band-tabs" role="tablist" aria-label="Distance bands"><button type="button" id="band-0" role="tab" aria-label="Band 0" aria-selected="true" tabindex="0" aria-controls="band-panel" data-band="0">0</button><button type="button" id="band-1" role="tab" aria-label="Band 1" aria-selected="false" tabindex="-1" aria-controls="band-panel" data-band="1">1</button><button type="button" id="band-2" role="tab" aria-label="Band 2" aria-selected="false" tabindex="-1" aria-controls="band-panel" data-band="2">2</button><button type="button" id="band-3" role="tab" aria-label="Band 3" aria-selected="false" tabindex="-1" aria-controls="band-panel" data-band="3">3</button><button type="button" id="band-4" role="tab" aria-label="Band 4" aria-selected="false" tabindex="-1" aria-controls="band-panel" data-band="4">4</button><button type="button" id="band-5" role="tab" aria-label="Band 5" aria-selected="false" tabindex="-1" aria-controls="band-panel" data-band="5">5</button><button type="button" id="band-6" role="tab" aria-label="Band 6" aria-selected="false" tabindex="-1" aria-controls="band-panel" data-band="6">6</button><button type="button" id="band-7" role="tab" aria-label="Band 7" aria-selected="false" tabindex="-1" aria-controls="band-panel" data-band="7">7</button><button type="button" id="band-8" role="tab" aria-label="Band 8" aria-selected="false" tabindex="-1" aria-controls="band-panel" data-band="8">8</button></div><div id="band-panel" role="tabpanel" aria-labelledby="band-0"><p id="band-draft" class="note" role="status"></p><p id="band-title">Band 0 · nearest</p><p id="tuning-distance" class="note">Target distance: waiting</p><button id="tuning-signals" type="button">Start live signals</button>
<div class="tuning-reading"><span>Movement signal</span><strong id="tuning-moving-value">—</strong></div><div class="energy-track" role="img" id="tuning-moving-track"><span id="tuning-moving-fill" class="energy-fill"></span><span id="tuning-moving-limit" class="energy-limit"></span></div><label for="moving-sensitivity">Movement <output id="moving-value"></output></label>
<input id="moving-sensitivity" name="moving" type="range" min="0" max="100" required>
<div class="tuning-reading"><span>Small movements signal</span><strong id="tuning-stationary-value">—</strong></div><div class="energy-track" role="img" id="tuning-stationary-track"><span id="tuning-stationary-fill" class="energy-fill"></span><span id="tuning-stationary-limit" class="energy-limit"></span></div><label for="stationary-sensitivity">Small movements <output id="stationary-value"></output></label>
<input id="stationary-sensitivity" name="stationary" type="range" min="0" max="100" required>
<p id="band-unavailable" class="note" hidden>Small movements cannot be adjusted in bands 0 and 1.</p><div class="slider-scale"><span>0 · more sensitive</span><span>100 · less sensitive</span></div><details><summary>What do these sliders change?</summary><p>Signal above the minimum = person detected. Set the minimum above empty-room signals but below signals from a person.</p></details><div class="actions"><button class="primary" type="submit">Save this band</button></div></div>
</fieldset>
</form></div><details><summary>How do distance bands work?</summary><p>Bands group signals by distance from the lamp.</p><figure><svg viewBox="0 0 480 260" role="img" aria-labelledby="bands-title bands-desc"><title id="bands-title">Distance bands spreading out from a lamp</title><desc id="bands-desc">A lamp on the left faces a room. Curved bands spread out from it. A nearby walking person and a farther seated person fall into different bands.</desc><defs><clipPath id="room-clip"><rect x="42" y="20" width="415" height="205" rx="8"/></clipPath></defs><rect x="42" y="20" width="415" height="205" rx="8" fill="#f0f6f6" stroke="#b6cbcf"/><g clip-path="url(#room-clip)" fill="none" stroke="#a1c9cc" stroke-width="24" opacity=".55"><circle cx="42" cy="122" r="44"/><circle cx="42" cy="122" r="88"/><circle cx="42" cy="122" r="132"/><circle cx="42" cy="122" r="176"/><circle cx="42" cy="122" r="220"/><circle cx="42" cy="122" r="264"/><circle cx="42" cy="122" r="308"/><circle cx="42" cy="122" r="352"/><circle cx="42" cy="122" r="396"/></g><rect x="24" y="99" width="32" height="46" rx="8" fill="#116d74"/><text x="8" y="168" font-size="15">Lamp</text><g stroke="#243e48" stroke-width="5" fill="none" stroke-linecap="round"><circle cx="165" cy="84" r="9"/><path d="M165 96v30m0-20l-18 15m18-15l18 7m-18 13l-14 24m14-24l17 23"/><circle cx="340" cy="92" r="9"/><path d="M340 104v27h24v24m-24-40h20m-29-9v34h28"/></g><text x="117" y="186" font-size="15">Nearby movement</text><text x="292" y="186" font-size="15">Farther away</text><path d="M83 241h347m-8-6l8 6-8 6" stroke="#526b75" fill="none"/><text x="116" y="258" font-size="14">Bands 0–8: increasing distance</text></svg><figcaption id="bands-caption">Illustration only. Band spacing can be 0.75 m or 0.2 m, depending on the radar setting. Use the live target distance below to check range.</figcaption></figure></details></section></div><div id="panel-testing" class="grid tab-panel" role="tabpanel" aria-labelledby="tab-testing" hidden><section aria-labelledby="lux-test-title"><h2 id="lux-test-title">Check the light sensor</h2><p id="lux-health" role="status">Waiting for sensor readings.</p><ol><li>Leave the sensor uncovered. Save a reading.</li><li>Cover the sensor with your hand. Save another reading.</li><li>Uncover it and check that Room light rises again.</li></ol><div class="actions"><button id="lux-open" type="button">Save uncovered reading</button><button id="lux-covered" type="button" disabled>Save covered reading</button></div><p id="lux-check" role="status">No readings saved yet.</p><details><summary>If the reading does not change</summary><p>Cover the light sensor, not the radar. Try a brighter room. If readings are missing, check its wiring with power disconnected.</p></details></section><section aria-labelledby="test-title">
<h2 id="test-title">Check the lights</h2><p>Choose a brightness, then test one light or all three in order.</p><label for="test-brightness">Test brightness <output id="test-brightness-value">25%</output></label><input id="test-brightness" type="range" min="1" max="100" value="25"><div class="preset-actions" aria-label="Test brightness presets"><button type="button" data-level="10">Low · 10%</button><button type="button" data-level="50">Medium · 50%</button><button type="button" data-level="100">Full · 100%</button></div><p id="leds" class="note">Checking</p>
<div class="test-actions"><button data-test="0">Run sequentially</button><button class="primary" data-test="4">All on</button></div>
<div class="individual-actions" aria-label="Individual LED tests"><button data-test="1">D1</button><button data-test="2">D2</button><button data-test="3">D3</button></div>
<div class="test-stop"><button class="danger" data-test="-1">Stop</button></div>
<p class="note">All on holds all three lights at the selected brightness. All on and individual tests stay on until you select Stop or another test. Stop returns to automatic lighting.</p>
<p id="test-message" class="message" role="status"></p>
</section><section class="wide" aria-labelledby="radar-title">
<h2 id="radar-title">Check people detection</h2>
<div class="radar-grid">
<div>
<dl>
<div class="reading"><dt>People detected</dt><dd id="presence">Checking</dd></div>
<div class="reading"><dt>Sensor connection</dt><dd id="radar-uart">Checking</dd></div>
<div class="reading"><dt>Target</dt><dd id="radar-target">—</dd></div>
<div class="reading"><dt>Moving target</dt><dd id="radar-moving">—</dd></div>
<div class="reading"><dt>Stationary target</dt><dd id="radar-stationary">—</dd></div>
</dl>
<details><summary>How do I test people detection?</summary><p class="note">Walk in, sit still, then leave. People should stay Detected while you are there, then change to Clear after the wait time.</p></details>

<div class="actions"><button id="radar-energy" type="button" aria-pressed="false">Show gate energies</button></div>
<p id="radar-message" class="message" role="status"></p>
</div>

</div>
<figure class="range-view" aria-labelledby="range-title">
<h3 id="range-title">Target range</h3>
<div class="range-row"><span>Moving</span><div class="range-track"><span id="moving-mark" class="range-mark" hidden></span></div></div>
<div class="range-row"><span>Stationary</span><div class="range-track"><span id="stationary-mark" class="range-mark stationary" hidden></span></div></div>
<div class="range-scale"><span>0 m</span><span id="range-mid">3 m</span><span id="range-end">6 m</span></div>
<p class="note">Distance from the lamp. A missing marker means no target is currently reported.</p>
</figure>
<div id="gate-view" class="gate-view" hidden>
<h3>Gate energies</h3>
<details><summary>How do I read these signals?</summary><p class="note">Bars show live signals. Marks show saved minimums. Signals above the marks count as people within your chosen range.</p></details>
<div id="gate-energies" class="gate-grid"></div>
</div>
</section></div><div id="panel-firmware" class="grid tab-panel" role="tabpanel" aria-labelledby="tab-firmware" hidden><section aria-labelledby="firmware-title">
<h2 id="firmware-title">Update your light</h2>
<form id="firmware">
<label for="image">Firmware file (-OTA.bin)</label>
<input id="image" name="firmware" type="file" accept=".bin" required>
<div class="actions"><button class="primary" id="firmware-submit" type="submit">Install update</button></div>
</form>
<details><summary>What happens during an update?</summary><p class="note">Choose the versioned -OTA.bin file and install. Keep power connected until the light restarts. Saved settings are kept.</p></details>
<p id="firmware-message" class="message" role="status"></p>
</section></div><div id="panel-wifi" class="grid tab-panel" role="tabpanel" aria-labelledby="tab-wifi" hidden><section><h2>Wi-Fi connection</h2>
<p>Enter your home Wi-Fi details. The light will check the connection, save and restart.</p>
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
<p class="note">To erase saved home Wi-Fi, hold BOOT until the blue LED flashes quickly (10 seconds), then release. Use BOOT, not RESET. Lamp and radar settings are kept. Do not hold BOOT while powering on.</p>

</section></div></main>
<script>
const token = document.querySelector('meta[name=csrf-token]').content;
const byId = id => document.getElementById(id);
const tabScroll = {}; let activeTab = 'settings';
function selectTab(name, focus = false) {
  if (name !== activeTab) {
    tabScroll[activeTab] = window.scrollY; activeTab = name;
    requestAnimationFrame(() => window.scrollTo(0, tabScroll[name] ?? document.querySelector('nav').offsetTop));
  }
  for (const tab of ['settings','testing','wifi','firmware']) {
    const selected = tab === name;
    byId('tab-' + tab).setAttribute('aria-selected', String(selected));
    byId('tab-' + tab).tabIndex = selected ? 0 : -1;
    byId('panel-' + tab).hidden = !selected;
  }
  if (name === 'settings' && !radarSettings && !pending && !radarBusy && !radarAwaiting) byId('radar-read').click();
  if (focus) byId('tab-' + name).focus({preventScroll:true});
}
for (const name of ['settings','testing','wifi','firmware']) {
  byId('tab-' + name).onpointerdown = event => { event.preventDefault(); event.currentTarget.focus({preventScroll:true}); };
  byId('tab-' + name).onclick = () => selectTab(name);
  byId('tab-' + name).onkeydown = event => {
    if (['ArrowLeft','ArrowRight','Home','End'].includes(event.key)) {
      event.preventDefault();
      const tabs = ['settings','testing','wifi','firmware'];
      selectTab(tabs[event.key === 'Home' ? 0 : event.key === 'End' ? 3 : (tabs.indexOf(name) + (event.key === 'ArrowLeft' ? 3 : 1)) % 4], true);
    }
  };
}
let liveLux = null; let uncoveredLux = null;
let pending = false;
let refreshing = false;
let nextStatusAt = 0;
let lastRadarAt = 0;
let loadedSettings = false;
let radarBusy = false;
let radarAwaiting = false;
let radarSettings = null;
const bandDrafts = {}; let rangeDirty = false;
let radarRevision = '';
let radarMessage = '';
let energyVisible = false;
let tuningRadar = null;
let autoSignalsRequested = false;
let energyColumns = [];
const targets = ['Clear', 'Moving', 'Stationary', 'Moving and stationary', 'Calibrating',
                 'Calibration complete', 'Calibration failed'];
const lightLevels = {
  below: 'Below threshold',
  above: 'At or above threshold',
  waiting: 'Waiting for a fresh reading',
  unavailable: 'Sensor unavailable - light off'
};
function message(id, text, error = false) {
  const el = byId(id + '-message');
  el.textContent = text;
  el.classList.toggle('error', error);
  if (id === 'radar') {
    byId('radar-settings-message').textContent = text;
    byId('radar-settings-message').classList.toggle('error', error);
  }
}
function syncControls() {
  document.querySelectorAll('button:not([role=tab])').forEach(button => button.disabled = pending);
  byId('radar-read').disabled = pending || radarBusy || radarAwaiting;
  byId('radar-energy').disabled = pending || radarBusy || radarAwaiting;
  byId('radar-range-fields').disabled = pending || radarBusy || radarAwaiting || !radarSettings;
  byId('radar-gate-fields').disabled = byId('radar-range-fields').disabled;
  byId('firmware-submit').disabled = pending || radarBusy || radarAwaiting;
}
function fillGate() {
  if (!radarSettings) return;
  const gate = Number(byId('radar-gate-select').value);
  const draft = bandDrafts[gate];
  byId('moving-sensitivity').value = draft ? draft.moving : radarSettings.moving[gate];
  byId('stationary-sensitivity').value = draft ? draft.stationary : radarSettings.stationary[gate];
  byId('stationary-sensitivity').disabled = gate < 2;
  byId('band-unavailable').hidden = gate >= 2;
  updateSliderValues();
}
function renderRange(radar) {
  const moving = radar.live && radar.target <= 3 && (radar.target & 1);
  const stationary = radar.live && radar.target <= 3 && (radar.target & 2);
  const end = Math.max(600, Math.ceil(Math.max(moving ? radar.movingCm : 0,
    stationary ? radar.stationaryCm : 0) / 100) * 100);
  byId('range-mid').textContent = end / 200 + ' m';
  byId('range-end').textContent = end / 100 + ' m';
  for (const [id, visible, distance] of [
    ['moving-mark', moving, radar.movingCm], ['stationary-mark', stationary, radar.stationaryCm]
  ]) {
    const mark = byId(id);
    mark.hidden = !visible;
    if (visible) mark.style.left = 100 * distance / end + '%';
  }
}
function addEnergy(column, label, stationary) {
  const reading = document.createElement('div');
  reading.className = 'energy-value';
  const name = document.createElement('span');
  name.textContent = label;
  const level = document.createElement('span');
  reading.append(name, level);
  const track = document.createElement('div');
  track.className = 'energy-track';
  track.setAttribute('role', 'img');
  const fill = document.createElement('span');
  fill.className = 'energy-fill' + (stationary ? ' stationary' : '');
  const limit = document.createElement('span');
  limit.className = 'energy-limit';
  track.append(fill, limit);
  column.append(reading, track);
  return {label, level, track, fill, limit};
}
function updateEnergy(bar, value, threshold) {
  bar.level.textContent = value ?? '—';
  bar.track.setAttribute('aria-label', bar.label + ': energy ' + (value ?? 'unavailable') +
    (threshold === undefined ? '' : ', threshold ' + threshold));
  bar.fill.hidden = value === undefined;
  if (value !== undefined) bar.fill.style.width = Math.min(100, value) + '%';
  bar.limit.hidden = threshold === undefined;
  if (threshold !== undefined) bar.limit.style.left = threshold + '%';
}
function renderEnergy(radar) {
  tuningRadar = radar;
  renderTuning();
  const count = Math.max(radar.movingGates.length, radar.stationaryGates.length);
  energyVisible = count > 0;
  byId('gate-view').hidden = !energyVisible;
  const button = byId('radar-energy');
  button.setAttribute('aria-pressed', String(energyVisible));
  button.textContent = energyVisible ? 'Hide gate energies' : 'Show gate energies';
  if (!energyVisible) return;
  if (energyColumns.length !== count) {
    energyColumns = Array.from({length: count}, (_, gate) => {
      const node = document.createElement('div');
      node.className = 'gate-energy';
      const label = document.createElement('strong');
      label.textContent = 'Gate ' + gate;
      node.append(label);
      return {node, moving: addEnergy(node, 'Moving', false), stationary: addEnergy(node, 'Stationary', true)};
    });
    byId('gate-energies').replaceChildren(...energyColumns.map(column => column.node));
  }
  energyColumns.forEach((column, gate) => {
    updateEnergy(column.moving, radar.movingGates[gate], radarSettings?.moving[gate]);
    updateEnergy(column.stationary, radar.stationaryGates[gate], gate >= 2 ? radarSettings?.stationary[gate] : undefined);
  });
}
function renderRadarReading(radar) {
  byId('presence').textContent = radar.presence === undefined ? 'Unknown' :
    radar.presence ? 'High · presence' : 'Low · clear';
  byId('radar-uart').textContent = radar.live ? 'Live · ' + radar.frames + ' frames' :
    radar.frames ? 'Reports paused' : radar.bytes ? 'Bytes received, no valid report' : 'No data';
  byId('radar-target').textContent = radar.live ? (targets[radar.target] || 'Unknown') : '—';
  byId('radar-moving').textContent = radar.live && radar.target <= 3 && (radar.target & 1) ?
    radar.movingCm + ' cm · energy ' + radar.movingEnergy : '—';
  byId('radar-stationary').textContent = radar.live && radar.target <= 3 && (radar.target & 2) ?
    radar.stationaryCm + ' cm · energy ' + radar.stationaryEnergy : '—';
  renderRange(radar);
  renderEnergy(radar);
}
function renderRadar(state) {
  const radar = state.radar;
  radarBusy = radar.busy;
  if (!radarBusy) {
    const revision = JSON.stringify(radar.settings);
    const reload = radarAwaiting || revision !== radarRevision;
    if (radar.message && (radarAwaiting || radar.message !== radarMessage)) {
      message('radar', radar.message, radar.error);
    }
    radarMessage = radar.message;
    radarAwaiting = false;
    radarSettings = radar.settings;
    if (radarSettings) {
      for (const [gate,draft] of Object.entries(bandDrafts)) {
        if (radarSettings.moving[gate] === draft.moving && radarSettings.stationary[gate] === draft.stationary) delete bandDrafts[gate];
      }
      if (Number(byId('moving-gate').value) === radarSettings.movingGate && Number(byId('stationary-gate').value) === radarSettings.stationaryGate && Number(byId('radar-absence').value) === radarSettings.absence) rangeDirty = false;
    }
    if (!radarSettings && !radar.message) message('radar', 'Settings not loaded. Select Reload radar settings. If loading fails, check radar power and UART wiring.', true);
    radarRevision = revision;
    if (reload && radarSettings) {
      byId('moving-gate').max = byId('stationary-gate').max = radarSettings.maxGate;
      if (!rangeDirty) {
      byId('moving-gate').value = radarSettings.movingGate;
      byId('stationary-gate').value = radarSettings.stationaryGate;
      byId('radar-absence').value = radarSettings.absence;
      }
      const select = byId('radar-gate-select');
      const lastGate = Math.min(Number(select.value), radarSettings.maxGate);
      document.querySelectorAll('[data-band]').forEach(button => button.disabled = Number(button.dataset.band) > radarSettings.maxGate);
      select.value = lastGate;
      fillGate();
    }
  }
  renderRadarReading(radar);
  lastRadarAt = performance.now();
  syncControls();
}
async function refresh() {
  if (pending || refreshing) return;
  refreshing = true;
  nextStatusAt = performance.now() + 1000;
  try {
    const response = await fetch('/status', {cache: 'no-store', signal: AbortSignal.timeout(1500)});
    if (!response.ok) throw new Error();
    const state = await response.json();
    renderRadar(state);
    liveLux = state.fresh && Number.isFinite(state.lux) ? state.lux : null;
    byId('lux-health').textContent = liveLux === null ? 'No fresh reading. Check the sensor connection.' : 'Sensor responding · ' + liveLux.toFixed(1) + ' lux';
    byId('brightness-value').textContent = byId('brightness').value + '%';
    byId('status-presence').textContent = state.presence ? 'Detected' : 'Clear';
    byId('lux').textContent = state.lux === null ? state.light : state.lux.toFixed(1) + ' lx' + (state.fresh ? '' : ' (last reading)');
    byId('lamp-now').textContent = state.leds.some(value => value > 0) ? 'On' : 'Off';

    byId('leds').textContent = state.leds.map((duty, i) => 'D' + (i + 1) + ': ' + (duty === null ? 'PWM error' : Math.round(duty * 100 / 255) + '%')).join(' / ');
    if (!loadedSettings) {
      for (const [name, value] of Object.entries(state.settings)) byId(name).value = value;
      loadedSettings = true;
    }
    message('status', '');
  } catch {
    liveLux = null; tuningRadar = null; renderTuning();
    byId('lux').textContent = byId('status-presence').textContent = byId('lamp-now').textContent = 'Offline';
    byId('lux-health').textContent = 'Connection lost. Waiting for fresh readings.';
    message('status', 'Connection lost. Retrying…', true);
  } finally {
    refreshing = false;
    if (radarSettings && !radarBusy && !pending && !radarAwaiting && !autoSignalsRequested) {
      autoSignalsRequested = true;
      submit('/radar-settings', 'radar', new URLSearchParams({action:'energy-on'}));
    }
  }
}
async function refreshRadar() {
  refreshing = true;
  try {
    const response = await fetch('/radar-live', {cache: 'no-store', signal: AbortSignal.timeout(1500)});
    if (!response.ok) throw new Error();
    renderRadarReading(await response.json());
    lastRadarAt = performance.now();
  } catch {
    if (performance.now() - lastRadarAt > 1500) {
      renderRadarReading({live: false, movingGates: [], stationaryGates: []});
      byId('radar-uart').textContent = 'Connection interrupted';
    }
  } finally {
    refreshing = false;
  }
}
function poll() {
  if (document.hidden || pending || refreshing) return;
  if (performance.now() >= nextStatusAt) refresh();
  else if (!byId('panel-testing').hidden || !byId('panel-settings').hidden) refreshRadar();
}
async function waitForRestart() {
  const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
  await sleep(4000);
  const deadline = Date.now() + 90000;
  while (Date.now() < deadline) {
    try {
      const response = await fetch('/status', {cache: 'no-store', signal: AbortSignal.timeout(2500)});
      if (response.ok) {
        location.reload();
        return true;
      }
    } catch {}
    await sleep(1500);
  }
  message('firmware', 'Firmware uploaded. Refresh once the nightlight rejoins Wi-Fi.', true);
  return false;
}
async function submit(path, id, data) {
  if (pending) return;
  pending = true;
  syncControls();
  if (id === 'firmware') message(id, 'Uploading firmware…');
  try {
    const response = await fetch(path + '?token=' + token, {method: 'POST', body: data, signal: AbortSignal.timeout(id === 'firmware' ? 180000 : 10000)});
    message(id, await response.text(), !response.ok);
    if (id === 'radar' && response.status === 202) radarAwaiting = true;
    if (id === 'firmware' && response.ok && await waitForRestart()) return;
  } catch {
    message(id, 'Connection lost. Check the device and refresh.', true);
  }
  pending = false;
  syncControls();
  if (id === 'settings' || id === 'radar') refresh();
}
document.querySelectorAll('[data-test]').forEach(button => button.onclick = () => {
  submit('/test', 'test', new URLSearchParams({channel: button.dataset.test, brightness: byId('test-brightness').value}));
});
byId('settings').onsubmit = event => {
  event.preventDefault();
  submit('/settings', 'settings', new URLSearchParams(new FormData(event.target)));
};
byId('radar-read').onclick = () => {
  submit('/radar-settings', 'radar', new URLSearchParams({action: 'read'}));
};
byId('radar-energy').onclick = () => {
  submit('/radar-settings', 'radar', new URLSearchParams({action: energyVisible ? 'energy-off' : 'energy-on'}));
};
byId('radar-gate-select').onchange = fillGate;
for (const action of ['range', 'gate']) {
  byId('radar-' + action).onsubmit = event => {
    event.preventDefault();
    const values = new URLSearchParams(new FormData(event.target));
    values.set('action', action);
    if (action === 'gate' && Number(byId('radar-gate-select').value) < 2) values.set('stationary', radarSettings.stationary[Number(byId('radar-gate-select').value)]);
    submit('/radar-settings', 'radar', values);
  };
}
byId('firmware').onsubmit = event => {
  event.preventDefault();
  submit('/update', 'firmware', new FormData(event.target));
};
function updateSliderValues() {
  renderTuning();
  for (const kind of ['moving','stationary']) byId(kind + '-value').textContent = byId(kind + '-sensitivity').value;
}
for (const kind of ['moving','stationary']) byId(kind + '-sensitivity').oninput = () => {
  const gate = Number(byId('radar-gate-select').value);
  bandDrafts[gate] = {moving:Number(byId('moving-sensitivity').value), stationary:Number(byId('stationary-sensitivity').value)};
  updateSliderValues();
};
for (const id of ['moving-gate','stationary-gate','radar-absence']) byId(id).oninput = () => { rangeDirty = true; };
function validateLightLevels() {
  const off = Number(byId('shutoff').value), on = Number(byId('darkness').value);
  byId('shutoff').setCustomValidity(off !== 0 && off <= on ? 'Choose a value above Turn on below, or use 0.' : '');
}
byId('shutoff').oninput = byId('darkness').oninput = validateLightLevels;
function renderTuning() {
  const gate = Number(byId('radar-gate-select').value);
  byId('band-draft').textContent = bandDrafts[gate] ? 'Unsaved changes in this band' : '';
  const radar = tuningRadar;
  const live = radar?.live;
  for (const kind of ['moving','stationary']) {
    const value = live ? radar[kind + 'Gates']?.[gate] : undefined;
    const minimum = Number(byId(kind + '-sensitivity').value);
    byId('tuning-' + kind + '-value').textContent = value === undefined ? 'No live signal' : value + ' / 100';
    byId('tuning-' + kind + '-fill').style.width = (value ?? 0) + '%';
    byId('tuning-' + kind + '-limit').style.left = minimum + '%';
    byId('tuning-' + kind + '-track').setAttribute('aria-label', kind + ' signal ' + (value ?? 'unavailable') + ', minimum ' + minimum);
  }
  const distances = [];
  if (live && radar.target <= 3 && (radar.target & 1)) distances.push('Moving ' + radar.movingCm + ' cm');
  if (live && radar.target <= 3 && (radar.target & 2)) distances.push('Still ' + radar.stationaryCm + ' cm');
  byId('tuning-distance').textContent = distances.join(' · ') || (live ? 'No target detected' : 'Waiting for radar');
  byId('tuning-signals').hidden = !!(live && radar.movingGates?.length);
}
byId('tuning-signals').onclick = () => submit('/radar-settings', 'radar', new URLSearchParams({action:'energy-on'}));
function selectBand(gate, focus = false) {
  byId('radar-gate-select').value = gate;
  document.querySelectorAll('[data-band]').forEach(button => {
    const selected = Number(button.dataset.band) === gate;
    button.setAttribute('aria-selected', String(selected)); button.tabIndex = selected ? 0 : -1;
  });
  byId('band-panel').setAttribute('aria-labelledby', 'band-' + gate);
  byId('band-title').textContent = 'Band ' + gate + (gate === 0 ? ' · nearest' : gate === 8 ? ' · farthest' : '');
  fillGate(); if (focus) byId('band-' + gate).focus();
}
document.querySelectorAll('[data-band]').forEach(button => {
  button.onclick = () => selectBand(Number(button.dataset.band));
  button.onkeydown = event => {
    if (!['ArrowLeft','ArrowRight','Home','End'].includes(event.key)) return;
    event.preventDefault(); const max = radarSettings?.maxGate ?? 8;
    selectBand(event.key === 'Home' ? 0 : event.key === 'End' ? max : (Number(button.dataset.band) + (event.key === 'ArrowLeft' ? max : 1)) % (max + 1), true);
  };
});
document.querySelectorAll('[data-page]').forEach(button => button.onclick = () => selectTab(button.dataset.page));
byId('brightness').oninput = () => byId('brightness-value').textContent = byId('brightness').value + '%';
byId('test-brightness').oninput = () => byId('test-brightness-value').textContent = byId('test-brightness').value + '%';
document.querySelectorAll('[data-level]').forEach(button => button.onclick = () => { byId('test-brightness').value = button.dataset.level; byId('test-brightness').oninput(); });
byId('lux-open').onclick = () => {
  if(liveLux === null) { byId('lux-check').textContent = 'Wait for a fresh sensor reading.'; return; }
  uncoveredLux = liveLux; byId('lux-covered').disabled = false;
  byId('lux-check').textContent = 'Uncovered: ' + uncoveredLux.toFixed(1) + ' lux. Cover the sensor, wait two seconds, then save the covered reading.';
};
byId('lux-covered').onclick = () => {
  if(liveLux === null || uncoveredLux === null) { byId('lux-check').textContent = 'Save a fresh uncovered reading first.'; return; }
  byId('lux-check').textContent = 'Uncovered: ' + uncoveredLux.toFixed(1) + ' lux. Covered: ' + liveLux.toFixed(1) + ' lux. ' + (liveLux < uncoveredLux ? 'The sensor responded to covering. Uncover it and check that the reading rises.' : 'No drop measured. Try again in a brighter room and check that you covered the light sensor.');
};
poll();
setTimeout(() => { if (!radarSettings) byId('radar-read').click(); }, 1700);
setInterval(poll, 100);

(()=>{
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
setInterval(() => { if (!saved && (checking || !byId('panel-wifi').hidden)) status(); }, 2000);
})();
if(location.hash === "#wifi") selectTab("wifi");
</script>
</body>
</html>)HTML";
