#pragma once

#include <WebServer.h>
#include <DNSServer.h>

#include "Lamp.h"
#include "Radar.h"

class Updater {
 public:
  explicit Updater(Radar &radar) : radar(radar) {}
  void begin(String (*readStatus)(), void (*testChannel)(int, uint8_t));
  void start();
  void stop();
  void resetWifi();
  void tick();
  void printNetworkStatus();
  void scanHome();
  void traceHome();
  bool configureWifi(const char *ssid, const char *passphrase, bool temporary);
  bool active() const { return listening; }
  bool busy() const { return receiving || rebootAt || wifiPending; }
  const LampSettings &settings() const { return lampSettings; }

 private:
  struct WifiConfig {
    char ssid[33];
    char passphrase[64];
  };

  struct NetworkConfig {
    WifiConfig wifi{};
    char ip[16]{};
    char gateway[16]{};
    char subnet[16]{};
    char dns[16]{};
  };

  void connectHome();
  bool applyNetwork(const NetworkConfig &config);
  bool saveNetwork(const NetworkConfig &config);
  void showWifi();
  void saveWifi();
  void wifiStatus();
  void scanWifi();
  void tickWifi();
  void listen();
  void finishTrace();
  bool authorized();
  bool requireAuthorization();
  void showPage();
  void showStatus();
  void handleTest();
  void saveSettings();
  void radarSettings();
  void receiveUpload();
  void finishUpload();

  WebServer server{80};
  DNSServer dnsServer;
  Radar &radar;
  NetworkConfig network{};
  NetworkConfig candidate{};
  char hostname[32]{};
  char apName[40]{};
  String wifiResult = "idle";
  String dashboardUrl;
  uint32_t wifiStartedAt = 0;
  uint32_t rebootDelay = 1000;
  bool wifiPending = false;
  bool wifiStarted = false;
  bool mdnsStarted = false;
  WifiConfig home{};
  LampSettings lampSettings{};
  String (*readStatus)() = nullptr;
  void (*testChannel)(int, uint8_t) = nullptr;
  char token[33]{};
  const char *uploadError = nullptr;
  uint32_t startedAt = 0;
  uint32_t connectAttemptAt = 0;
  uint32_t traceStartedAt = 0;
  uint32_t rebootAt = 0;
  bool listening = false;
  bool hotspot = false;
  bool homeConnected = false;
  bool testingWifi = false;
  bool receiving = false;
  bool complete = false;
  bool checkedImage = false;
};
