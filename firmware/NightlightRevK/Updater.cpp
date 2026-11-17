#include "Updater.h"

#include <Arduino.h>
#include <Preferences.h>
#include <Update.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_random.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>

#include "Dashboard.h"
#include "WifiPage.h"
#include "NetworkValidation.h"

namespace {
constexpr uint32_t hotspotWindowMs = 5 * 60 * 1000;
constexpr uint32_t reconnectMs = 20000;
constexpr uint32_t traceMs = 12000;
constexpr char lampSettingsKey[] = "settings-v3";

String jsonString(const String &value) {
  String result = "\"";
  for (size_t i = 0; i < value.length(); ++i) {
    const uint8_t c = value[i];
    if (c == '"' || c == '\\') { result += '\\'; result += char(c); }
    else if (c < 0x20) {
      char escaped[7];
      snprintf(escaped, sizeof(escaped), "\\u%04x", unsigned(c));
      result += escaped;
    } else result += char(c);
  }
  return result + '"';
}

struct WifiTrace {
  uint32_t beacons = 0;
  uint32_t authentication = 0;
  uint32_t association = 0;
  uint32_t deauthentication = 0;
  uint32_t disassociation = 0;
  uint16_t authStatus = 0;
  uint16_t deauthReason = 0;
};
portMUX_TYPE traceLock = portMUX_INITIALIZER_UNLOCKED;
WifiTrace trace;
uint8_t stationMac[6]{};

void receiveManagement(void *buffer, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT) return;
  const auto *packet = static_cast<const wifi_promiscuous_pkt_t *>(buffer);
  if (packet->rx_ctrl.sig_len < 24) return;
  const uint8_t *frame = packet->payload;
  const uint8_t subtype = frame[0] & 0xfc;
  const bool beacon = subtype == 0x80;
  if (!beacon && memcmp(frame + 4, stationMac, sizeof(stationMac)) != 0) return;
  portENTER_CRITICAL(&traceLock);
  if (beacon) {
    ++trace.beacons;
  } else if (subtype == 0xb0 && packet->rx_ctrl.sig_len >= 30) {
    ++trace.authentication;
    trace.authStatus = frame[28] | (uint16_t(frame[29]) << 8);
  } else if (subtype == 0x10 || subtype == 0x30) {
    ++trace.association;
  } else if (subtype == 0xc0) {
    ++trace.deauthentication;
    if (!(frame[1] & 0x40) && packet->rx_ctrl.sig_len >= 26) {
      trace.deauthReason = frame[24] | (uint16_t(frame[25]) << 8);
    }
  } else if (subtype == 0xa0) {
    ++trace.disassociation;
  }
  portEXIT_CRITICAL(&traceLock);
}

void randomHex(char *out, size_t digits) {
  static constexpr char hex[] = "0123456789abcdef";
  uint8_t bytes[16];
  esp_fill_random(bytes, digits / 2);
  for (size_t i = 0; i < digits / 2; ++i) {
    out[2 * i] = hex[bytes[i] >> 4];
    out[2 * i + 1] = hex[bytes[i] & 0xf];
  }
  out[digits] = '\0';
}

bool readNumber(const String &value, uint16_t &number) {
  if (value.isEmpty() || value.length() > 5) return false;
  for (size_t i = 0; i < value.length(); ++i) {
    if (value[i] < '0' || value[i] > '9') return false;
  }
  const long parsed = value.toInt();
  if (parsed > UINT16_MAX) return false;
  number = parsed;
  return true;
}

bool readDarkness(const String &value, uint16_t &tenths) {
  const int point = value.indexOf('.');
  uint16_t whole = 0;
  uint8_t fraction = 0;
  if (point < 0) {
    if (!readNumber(value, whole)) return false;
  } else {
    if (value.length() != size_t(point + 2)) return false;
    const char digit = value[point + 1];
    if (digit < '0' || digit > '9') return false;
    fraction = digit - '0';
    if (point > 0 && !readNumber(value.substring(0, point), whole)) return false;
  }
  const uint32_t scaled = uint32_t(whole) * 10 + fraction;
  if (scaled > LampSettings::maxDarknessTenths) return false;
  tenths = scaled;
  return true;
}

LampSettings loadLampSettings(Preferences &prefs) {
  LampSettings saved;
  if (prefs.getBytesLength(lampSettingsKey) == sizeof(saved) &&
      prefs.getBytes(lampSettingsKey, &saved, sizeof(saved)) == sizeof(saved) && saved.valid()) {
    return saved;
  }
  struct Legacy { uint16_t brightness, darkness, hold; } old{};
  for (const char *key : {"settings-v2", "settings-v1"}) {
    if (prefs.getBytesLength(key) != sizeof(old) || prefs.getBytes(key, &old, sizeof(old)) != sizeof(old)) continue;
    const uint32_t darkness = uint32_t(old.darkness) * (strcmp(key, "settings-v1") == 0 ? 10 : 1);
    if (darkness > LampSettings::maxDarknessTenths) continue;
    saved.brightness = old.brightness;
    saved.darknessTenths = darkness;
    saved.holdSeconds = old.hold;
    if (saved.valid()) return saved;
  }
  return LampSettings{};
}
}

void Updater::begin(String (*statusProvider)(), void (*testControl)(int, uint8_t)) {
  readStatus = statusProvider;
  testChannel = testControl;
  Preferences prefs;
  if (prefs.begin("nightlight")) {
    if (prefs.getBytesLength("home-wifi") == sizeof(home)) {
      prefs.getBytes("home-wifi", &home, sizeof(home));
    }
    network.wifi = home;
    if (prefs.getBytesLength("network-v2") == sizeof(network)) {
      prefs.getBytes("network-v2", &network, sizeof(network));
      home = network.wifi;
    }
    home.ssid[32] = home.passphrase[63] = '\0';
    network.wifi = home;
    network.ip[15] = network.gateway[15] = network.subnet[15] = network.dns[15] = '\0';
    lampSettings = loadLampSettings(prefs);
    prefs.end();
  }
  WiFi.persistent(false);
  WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t info) {
    const auto &event = info.wifi_sta_disconnected;
    const auto reason = static_cast<wifi_err_reason_t>(event.reason);
    Serial.printf("Wi-Fi disconnected: %s (%u) | AP %02X:%02X:%02X:%02X:%02X:%02X | RSSI %d dBm\n",
                  WiFi.disconnectReasonName(reason), unsigned(reason), event.bssid[0], event.bssid[1],
                  event.bssid[2], event.bssid[3], event.bssid[4], event.bssid[5], int(event.rssi));
  }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  const uint32_t id = uint32_t(ESP.getEfuseMac() >> 24) & 0xffffff;
  snprintf(hostname, sizeof(hostname), "nightlight-%06lx", (unsigned long)id);
  snprintf(apName, sizeof(apName), "Nightlight-%06lx", (unsigned long)id);
  connectHome();
  server.on("/", HTTP_GET, [this] { if (hotspot && server.client().localIP() == WiFi.softAPIP()) showWifi(); else showPage(); });
  server.on("/dashboard", HTTP_GET, [this] { showPage(); });
  server.on("/wifi", HTTP_GET, [this] { showWifi(); });
  server.on("/wifi", HTTP_POST, [this] { saveWifi(); });
  server.on("/wifi-status", HTTP_GET, [this] { wifiStatus(); });
  server.on("/wifi-scan", HTTP_POST, [this] { scanWifi(); });
  server.onNotFound([this] {
    server.sendHeader("Location", hotspot ? "http://192.168.4.1/" : "/");
    server.send(302, "text/plain", "Open Nightlight setup.");
  });
  server.on("/status", HTTP_GET, [this] { showStatus(); });
  server.on("/radar-live", HTTP_GET, [this] {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", radar.json(millis(), false));
  });
  server.on("/test", HTTP_POST, [this] { handleTest(); });
  server.on("/settings", HTTP_POST, [this] { saveSettings(); });
  server.on("/radar-settings", HTTP_POST, [this] { radarSettings(); });
  server.on("/update", HTTP_POST, [this] { finishUpload(); }, [this] { receiveUpload(); });
  if (!home.ssid[0]) start();
}

void Updater::connectHome() {
  if (!home.ssid[0]) return;
  WiFi.mode(hotspot ? WIFI_AP_STA : WIFI_STA);
  WiFi.setHostname(hostname);
  if (!applyNetwork(network)) {
    Serial.println("Invalid saved network configuration. Hold BOOT for 10 seconds and release to reset Wi-Fi.");
    return;
  }
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.begin(home.ssid, home.passphrase);
  connectAttemptAt = millis();
}

bool Updater::configureWifi(const char *ssid, const char *passphrase, bool temporary) {
  if (busy() || !ssid || !passphrase || !ssid[0] || strlen(ssid) > 32 ||
      strlen(passphrase) < 8 || strlen(passphrase) > 63) return false;
  WifiConfig config{};
  strlcpy(config.ssid, ssid, sizeof(config.ssid));
  strlcpy(config.passphrase, passphrase, sizeof(config.passphrase));
  NetworkConfig next{};
  next.wifi = config;
  if (!temporary && !saveNetwork(next)) return false;
  network = next;
  testingWifi = temporary;
  home = config;
  homeConnected = false;
  stop();
  if (listening) {
    server.stop();
    listening = false;
  }
  WiFi.disconnect();
  connectHome();
  return true;
}

void Updater::printNetworkStatus() {
  const char *label = testingWifi ? "Test Wi-Fi" : "Home Wi-Fi";
  if (!home.ssid[0]) {
    Serial.println("Home Wi-Fi: unconfigured");
  } else if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("%s: %s | IP: %s | MAC: %s\n",
                  label, home.ssid, WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
    Serial.printf("Access point: %s | channel %d | RSSI %ld dBm\n",
                  WiFi.BSSIDstr().c_str(), int(WiFi.channel()), long(WiFi.RSSI()));
  } else {
    Serial.printf("%s: connecting to %s\n", label, home.ssid);
  }
}

void Updater::scanHome() {
  if (!home.ssid[0] || busy() || hotspot) {
    Serial.println("Home Wi-Fi scan unavailable.");
    return;
  }
  Serial.printf("Scanning for %s on 2.4 GHz…\n", home.ssid);
  WiFi.setAutoReconnect(false);
  WiFi.disconnect();
  delay(150);
  const int count = WiFi.scanNetworks(false, true, false, 120);
  int matches = 0;
  for (int i = 0; i < count && i < 256; ++i) {
    if (WiFi.SSID(i) != home.ssid) continue;
    ++matches;
    Serial.printf("Home Wi-Fi candidate: %s | channel %d | RSSI %ld dBm | security %u\n",
                  WiFi.BSSIDstr(i).c_str(), int(WiFi.channel(i)), long(WiFi.RSSI(i)),
                  unsigned(WiFi.encryptionType(i)));
  }
  Serial.printf("Home Wi-Fi scan: %d matching access point(s), scan result %d\n", matches, count);
  WiFi.scanDelete();
  connectHome();
}

void Updater::traceHome() {
  if (!home.ssid[0] || busy() || hotspot || traceStartedAt || WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi trace is available while home Wi-Fi is disconnected.");
    return;
  }
  const wifi_promiscuous_filter_t filter{WIFI_PROMIS_FILTER_MASK_MGMT};
  if (esp_wifi_get_mac(WIFI_IF_STA, stationMac) != ESP_OK ||
      esp_wifi_set_promiscuous_filter(&filter) != ESP_OK ||
      esp_wifi_set_promiscuous_rx_cb(receiveManagement) != ESP_OK ||
      esp_wifi_set_promiscuous(true) != ESP_OK) {
    Serial.println("Could not start Wi-Fi trace.");
    return;
  }
  portENTER_CRITICAL(&traceLock);
  trace = {};
  portEXIT_CRITICAL(&traceLock);
  traceStartedAt = connectAttemptAt = millis();
  Serial.println("Tracing Wi-Fi join for 12 seconds.");
  WiFi.reconnect();
}

void Updater::finishTrace() {
  esp_wifi_set_promiscuous(false);
  portENTER_CRITICAL(&traceLock);
  const WifiTrace result = trace;
  portEXIT_CRITICAL(&traceLock);
  traceStartedAt = 0;
  Serial.printf("Wi-Fi trace: %lu beacons | %lu auth replies (last status %u) | %lu assoc replies | "
                "%lu deauth (last reason %u) | %lu disassoc\n",
                (unsigned long)result.beacons, (unsigned long)result.authentication, unsigned(result.authStatus),
                (unsigned long)result.association, (unsigned long)result.deauthentication,
                unsigned(result.deauthReason), (unsigned long)result.disassociation);
}

void Updater::start() {
  if (busy()) return;
  if (hotspot) { startedAt = millis(); return; }
  if (!WiFi.mode(WIFI_AP_STA) ||
      !WiFi.softAP(apName, nullptr, 1, false, 2)) {
    Serial.println("Could not start fallback Wi-Fi.");
    WiFi.mode(home.ssid[0] ? WIFI_STA : WIFI_OFF);
    return;
  }
  hotspot = true;
  dnsServer.start(53, "*", WiFi.softAPIP());
  startedAt = millis();
  listen();
  Serial.printf("Wi-Fi setup at http://%s/%s\n",
                WiFi.softAPIP().toString().c_str(), home.ssid[0] ? " for five minutes." : " until configured.");
}

void Updater::listen() {
  if (listening) return;
  randomHex(token, sizeof(token) - 1);
  server.begin();
  listening = true;
}

void Updater::stop() {
  if (!hotspot || busy()) return;
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(home.ssid[0] ? WIFI_STA : WIFI_OFF);
  hotspot = false;
  Serial.println("Fallback Wi-Fi closed.");
}

void Updater::tick() {
  if (rebootAt && millis() - rebootAt >= rebootDelay) ESP.restart();
  if (hotspot) dnsServer.processNextRequest();
  tickWifi();
  if (traceStartedAt && millis() - traceStartedAt >= traceMs) finishTrace();
  const bool connected = home.ssid[0] && WiFi.status() == WL_CONNECTED;
  if (connected && !homeConnected) {
    printNetworkStatus();
    if (mdnsStarted) MDNS.end();
    mdnsStarted = MDNS.begin(hostname);
    if (mdnsStarted) MDNS.addService("http", "tcp", 80);
  }
  homeConnected = connected;
  if (!wifiPending && home.ssid[0] && !connected && !hotspot && !WiFi.STA.connected() &&
      millis() - connectAttemptAt >= reconnectMs) {
    connectAttemptAt = millis();
    WiFi.reconnect();
  }
  if (connected || hotspot) {
    listen();
    if (listening) server.handleClient();
  } else if (listening && !busy()) {
    server.stop();
    listening = false;
  }
  if (hotspot && home.ssid[0] && !wifiPending && millis() - startedAt >= hotspotWindowMs) stop();
}

bool Updater::authorized() {
  return listening && !rebootAt && server.arg("token") == token;
}

bool Updater::requireAuthorization() {
  if (busy()) {
    server.send(409, "text/plain", "Wait for the current operation to finish.");
    return false;
  }
  if (authorized()) return true;
  server.send(403, "text/plain", "Reload the dashboard and try again.");
  return false;
}

void Updater::showPage() {
  String body(FPSTR(dashboardPage));
  body.replace("{{token}}", token);
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Referrer-Policy", "no-referrer");
  server.send(200, "text/html; charset=utf-8", body);
}

void Updater::showStatus() {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", readStatus());
}

void Updater::handleTest() {
  if (!requireAuthorization()) return;
  const String value = server.arg("channel");
  if (value == "-1") {
    testChannel(-1, 25);
    server.send(200, "text/plain", "LED test stopped.");
  } else if (value.length() == 1 && value[0] >= '0' && value[0] <= '4') {
    uint16_t brightness = 25;
    if (server.hasArg("brightness") && (!readNumber(server.arg("brightness"), brightness) || brightness < 1 || brightness > 100)) {
      server.send(400, "text/plain", "Choose test brightness from 1 to 100 percent."); return;
    }
    testChannel(value[0] - '0', brightness);
    server.send(200, "text/plain", "LED test started. Check the lights on the board.");
  } else {
    server.send(400, "text/plain", "Choose an LED, Run sequentially, or All on.");
  }
}

void Updater::saveSettings() {
  if (!requireAuthorization()) return;
  LampSettings next;
  if (!readNumber(server.arg("brightness"), next.brightness) ||
      !readDarkness(server.arg("darkness"), next.darknessTenths) ||
      !readDarkness(server.arg("shutoff"), next.shutoffTenths) ||
      !readNumber(server.arg("holdSeconds"), next.holdSeconds) || !next.valid()) {
    server.send(400, "text/plain", "Enter values within the ranges shown. Shutoff must be 0 (disabled) or higher than the darkness threshold.");
    return;
  }
  Preferences prefs;
  if (!prefs.begin("nightlight") || prefs.putBytes(lampSettingsKey, &next, sizeof(next)) != sizeof(next)) {
    server.send(500, "text/plain", "Could not save settings.");
    return;
  }
  lampSettings = next;
  server.send(200, "text/plain", "Settings saved.");
}

void Updater::radarSettings() {
  if (!requireAuthorization()) return;
  if (radar.busy()) {
    server.send(409, "text/plain", "A radar settings operation is in progress.");
    return;
  }
  const String action = server.arg("action");
  uint16_t moving, stationary, third;
  const uint32_t now = millis();
  bool accepted = false;
  if (action == "read") {
    accepted = radar.readSettings(now);
  } else if (action == "energy-on" || action == "energy-off") {
    accepted = radar.setEnergyReports(action == "energy-on", now);
  } else if ((action == "range" || action == "gate") &&
             readNumber(server.arg("moving"), moving) &&
             readNumber(server.arg("stationary"), stationary) &&
             readNumber(server.arg(action == "range" ? "absence" : "gate"), third)) {
    accepted = action == "range" ? radar.setRange(moving, stationary, third, now) :
                                   radar.setGate(third, moving, stationary, now);
  }
  if (accepted) {
    server.send(202, "text/plain", "Waiting for the radar…");
  } else {
    server.send(400, "text/plain", "Read the radar settings first and enter values within the shown ranges.");
  }
}

void Updater::receiveUpload() {
  if (!authorized()) return;
  HTTPUpload &upload = server.upload();
  switch (upload.status) {
    case UPLOAD_FILE_START:
      if (receiving) Update.abort();
      receiving = false;
      complete = checkedImage = false;
      uploadError = nullptr;
      if (wifiPending || rebootAt || radar.busy()) {
        uploadError = "Wait for the radar settings operation to finish.";
      } else if (upload.name != "firmware" || !upload.filename.endsWith(".bin")) {
        uploadError = "Choose the ota.bin update file.";
      } else if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
        uploadError = Update.errorString();
      } else {
        receiving = true;
        testChannel(-1, 25);
      }
      break;
    case UPLOAD_FILE_WRITE:
      if (radar.busy()) radar.tick(millis());
      if (esp_task_wdt_status(nullptr) == ESP_OK) esp_task_wdt_reset();
      if (!receiving) break;
      if (!checkedImage) {
        // The application descriptor follows the 24-byte image and 8-byte segment headers.
        const uint8_t *bytes = upload.buf;
        if (upload.currentSize < 36 || bytes[0] != 0xe9 || bytes[32] != 0x32 ||
            bytes[33] != 0x54 || bytes[34] != 0xcd || bytes[35] != 0xab) {
          uploadError = "Invalid application binary. Choose the app-only .bin.";
        }
        checkedImage = true;
      }
      if (!uploadError && Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        uploadError = Update.errorString();
      }
      if (uploadError) {
        Update.abort();
        receiving = false;
      }
      break;
    case UPLOAD_FILE_END:
      complete = receiving && checkedImage;
      break;
    case UPLOAD_FILE_ABORTED:
      if (receiving) Update.abort();
      receiving = complete = false;
      uploadError = "Upload interrupted.";
      break;
  }
}

void Updater::finishUpload() {
  if (!authorized()) { server.send(403, "text/plain", "Reload and retry."); return; }
  const bool success = complete && Update.end(true);
  if (!success && !uploadError) {
    uploadError = complete ? Update.errorString() : "No complete application file received.";
  }
  if (receiving && !success) Update.abort();
  receiving = complete = false;
  server.sendHeader("Connection", "close");
  if (success) {
    server.send(200, "text/plain", "Firmware installed. Nightlight is restarting.");
    rebootAt = millis();
  } else {
    server.send(400, "text/plain", uploadError);
  }
}


bool Updater::applyNetwork(const NetworkConfig &config) {
  if (!WiFi.STA.begin()) return false;
  if (!config.ip[0]) return WiFi.STA.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0));
  IPAddress ip, gateway, subnet, dns;
  if (!ip.fromString(config.ip) || !gateway.fromString(config.gateway) ||
      !subnet.fromString(config.subnet) || !dns.fromString(config.dns)) return false;
  return WiFi.STA.config(ip, gateway, subnet, dns);
}

bool Updater::saveNetwork(const NetworkConfig &config) {
  Preferences prefs;
  return prefs.begin("nightlight") &&
         prefs.putBytes("network-v2", &config, sizeof(config)) == sizeof(config);
}

void Updater::resetWifi() {
  if (busy()) return;
  NetworkConfig empty{};
  if (!saveNetwork(empty)) {
    Serial.println("Wi-Fi reset failed: storage unavailable.");
    return;
  }
  // The empty v2 record takes precedence over legacy credentials even after power loss.
  Preferences prefs;
  if (prefs.begin("nightlight")) prefs.remove("home-wifi");
  home = empty.wifi;
  network = empty;
  WiFi.disconnect();
  homeConnected = false;
  if (mdnsStarted) { MDNS.end(); mdnsStarted = false; }
  wifiResult = "idle";
  dashboardUrl = "";
  start();
  Serial.println("Home Wi-Fi erased. Join the open setup hotspot; no password is required.");
}

void Updater::showWifi() {
  server.sendHeader("Location", "/dashboard#wifi");
  server.send(302, "text/plain", "Open Wi-Fi settings.");
}

void Updater::wifiStatus() {
  server.sendHeader("Cache-Control", "no-store");
  String body = "{\"state\":\"" + wifiResult + "\",\"url\":\"" + dashboardUrl +
    "\",\"hostname\":\"http://" + hostname + ".local/\",\"hotspot\":" +
    (hotspot && server.client().localIP() == WiFi.softAPIP() ? "true" : "false") + "}";
  server.send(200, "application/json", body);
}

void Updater::saveWifi() {
  if (!requireAuthorization()) return;
  const String ssid = server.arg("ssid"), pass = server.arg("password");
  if (ssid.isEmpty() || ssid.length() > 32 || pass.length() < 8 || pass.length() > 63 ||
      strlen(ssid.c_str()) != ssid.length() || strlen(pass.c_str()) != pass.length()) {
    server.send(400, "text/plain", "Use a 1-32 byte network name and an 8-63 byte WPA2/WPA3 passphrase.");
    return;
  }
  NetworkConfig next{};
  ssid.toCharArray(next.wifi.ssid, sizeof(next.wifi.ssid));
  pass.toCharArray(next.wifi.passphrase, sizeof(next.wifi.passphrase));
  if (server.arg("addressing") == "static") {
    IPAddress ip, gateway, subnet, dns;
    auto number = [](const IPAddress &a) {
      return (uint32_t(a[0]) << 24) | (uint32_t(a[1]) << 16) | (uint32_t(a[2]) << 8) | a[3];
    };
    const bool valid = ip.fromString(server.arg("ip")) && gateway.fromString(server.arg("gateway")) &&
      subnet.fromString(server.arg("subnet")) && dns.fromString(server.arg("dns")) &&
      ip.type() == IPv4 && gateway.type() == IPv4 && subnet.type() == IPv4 && dns.type() == IPv4 &&
      NetworkValidation::staticAddress(number(ip), number(gateway), number(subnet), number(dns));
    if (!valid) {
      server.send(400, "text/plain", "Use valid IPv4 addresses, a contiguous /1-/30 subnet, and a gateway in the same subnet. The home subnet must not overlap setup network 192.168.4.0/24.");
      return;
    }
    ip.toString().toCharArray(next.ip, sizeof(next.ip));
    gateway.toString().toCharArray(next.gateway, sizeof(next.gateway));
    subnet.toString().toCharArray(next.subnet, sizeof(next.subnet));
    dns.toString().toCharArray(next.dns, sizeof(next.dns));
  } else if (server.arg("addressing") != "dhcp") {
    server.send(400, "text/plain", "Choose automatic or manual addressing.");
    return;
  }
  candidate = next;
  if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) esp_wifi_scan_stop();
  WiFi.scanDelete();
  wifiPending = true;
  wifiStarted = false;
  wifiStartedAt = millis();
  wifiResult = "connecting";
  dashboardUrl = "";
  server.send(202, "text/plain", "Checking Wi-Fi. If this page disconnects, join the selected network and open the lamp's dashboard link. If connection fails, the lamp returns to its previous network.");
}

void Updater::scanWifi() {
  if (!requireAuthorization()) return;
  startedAt = millis();
  int count = WiFi.scanComplete();
  if (server.arg("refresh") == "1" && count != WIFI_SCAN_RUNNING) {
    WiFi.scanDelete();
    count = WiFi.scanNetworks(true, false, false, 120);
  }
  server.sendHeader("Cache-Control", "no-store");
  if (count == WIFI_SCAN_RUNNING) {
    server.send(202, "application/json", "{\"state\":\"scanning\"}");
    return;
  }
  if (count < 0) {
    server.send(503, "text/plain", "Could not scan Wi-Fi. Select Refresh networks to retry.");
    return;
  }
  String body = "{\"state\":\"ready\",\"networks\":[";
  bool first = true;
  for (int i = 0; i < count && i < 64; ++i) {
    const String ssid = WiFi.SSID(i);
    if (ssid.isEmpty()) continue;
    const auto auth = WiFi.encryptionType(i);
    const bool supported = auth == WIFI_AUTH_WPA2_PSK || auth == WIFI_AUTH_WPA_WPA2_PSK ||
      auth == WIFI_AUTH_WPA3_PSK || auth == WIFI_AUTH_WPA2_WPA3_PSK;
    if (!first) body += ',';
    first = false;
    body += "{\"ssid\":" + jsonString(ssid) + ",\"rssi\":" + String(WiFi.RSSI(i)) +
      ",\"supported\":" + (supported ? "true" : "false") + "}";
  }
  body += "]}";
  server.send(200, "application/json", body);
}

void Updater::tickWifi() {
  if (!wifiPending) return;
  if (!wifiStarted) {
    if (millis() - wifiStartedAt < 750) return;
    WiFi.setAutoReconnect(false);
    WiFi.disconnect();
    WiFi.mode(hotspot ? WIFI_AP_STA : WIFI_STA);
    WiFi.setHostname(hostname);
    if (!applyNetwork(candidate)) {
      wifiResult = "invalid";
      wifiPending = false;
      connectHome();
      return;
    }
    WiFi.begin(candidate.wifi.ssid, candidate.wifi.passphrase);
    wifiStarted = true;
    wifiStartedAt = millis();
    return;
  }
  if (WiFi.status() == WL_CONNECTED && uint32_t(WiFi.localIP()) != 0) {
    if (saveNetwork(candidate)) {
      network = candidate;
      home = candidate.wifi;
      dashboardUrl = "http://" + WiFi.localIP().toString() + "/";
      wifiResult = "saved";
      rebootDelay = 15000;
      rebootAt = millis();
    } else {
      startedAt = millis();
      wifiResult = "storage-error";
      WiFi.disconnect();
      connectHome();
    }
    wifiPending = false;
    candidate = {};
  } else if (millis() - wifiStartedAt >= 45000) {
    startedAt = millis();
    wifiResult = "failed";
    wifiPending = false;
    candidate = {};
    WiFi.disconnect();
    connectHome();
  }
}

