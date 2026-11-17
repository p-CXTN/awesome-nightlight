#include <Arduino.h>
#include <Wire.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>

#include "Lamp.h"
#include "RecoveryFeedback.h"
#include "Radar.h"
#include "Updater.h"

namespace {
constexpr uint8_t ledPins[] = {10, 0, 1};
constexpr uint8_t radarOut = 3;
constexpr uint8_t radarRx = 6;
constexpr uint8_t radarTx = 7;
constexpr uint8_t bootButton = 9;
constexpr uint8_t blueLed = 8;  // SuperMini onboard LED is active-low.
constexpr uint8_t sda = 4;
constexpr uint8_t scl = 5;
constexpr uint8_t lightAddress = 0x23;
constexpr uint32_t radarDebounceMs = 75;
constexpr uint32_t wifiResetHoldMs = 10000;
constexpr uint32_t testStepMs = 1500;
uint8_t testDuty = 64;

Lamp lamp;
Radar radar(Serial1);
Updater updater(radar);
bool pwmReady[3]{};
uint8_t appliedDuty[3]{};
bool radarRaw = false;
bool presence = false;
uint32_t radarChangedAt = 0;
bool buttonDown = false;
bool buttonHandled = false;
uint32_t buttonDownAt = 0;
bool watchdogEnabled = false;

enum class LightState { Checking, Ready, Unavailable };
LightState lightState = LightState::Checking;
bool measuring = false;
bool sampleFresh = false;
uint32_t lightChangedAt = 0;
uint32_t lightInterval = 0;
uint32_t luxTenths = 0;

int testFirst = -1;
bool testAllOn = false;
int testLast = -1;
int reportedChannel = -1;
uint32_t testStartedAt = 0;
char command[65];
size_t commandSize = 0;
bool commandTooLong = false;
enum class WifiInput { Idle, Ssid, Passphrase };
WifiInput wifiInput = WifiInput::Idle;
char newSsid[33]{};
bool temporaryWifi = false;

void writeLed(size_t channel, uint8_t duty) {
  if (pwmReady[channel] && appliedDuty[channel] != duty) {
    ledcWrite(ledPins[channel], duty);
    appliedDuty[channel] = duty;
  }
}

void sampleLight(uint32_t now, bool allowed) {
  if (!allowed) {
    measuring = sampleFresh = false;
    lightChangedAt = now;
    lightInterval = 0;
    return;
  }
  if (now - lightChangedAt < lightInterval) return;
  lightChangedAt = now;

  if (measuring) {
    measuring = false;
    if (Wire.requestFrom(lightAddress, uint8_t(2)) == 2) {
      const uint16_t reading = (uint16_t(Wire.read()) << 8) | uint16_t(Wire.read());
      // Convert raw / 1.2 lux to tenths, rounded to the nearest tenth.
      luxTenths = (uint32_t(reading) * 25 + 1) / 3;
      sampleFresh = true;
      lightState = LightState::Ready;
      lightInterval = 100;
      return;
    }
  } else {
    Wire.beginTransmission(lightAddress);
    Wire.write(uint8_t(0x20));  // One-time high-resolution measurement.
    if (Wire.endTransmission() == 0) {
      measuring = true;
      lightInterval = 180;
      return;
    }
  }
  sampleFresh = false;
  lightState = LightState::Unavailable;
  lightInterval = 1000;
}

void readRadar(uint32_t now) {
  radar.tick(now);
  if (radar.busy()) return;
  const bool raw = digitalRead(radarOut);
  if (raw != radarRaw) {
    radarRaw = raw;
    radarChangedAt = now;
  }
  if (presence != radarRaw && now - radarChangedAt >= radarDebounceMs) {
    presence = radarRaw;
    radar.setPresence(presence);
    Serial.printf("Radar OUT: %s\n", presence ? "HIGH (presence)" : "LOW");
  }
}

void showHelp() {
  Serial.println("Commands: test | test 1 | test 2 | test 3 | status | stop | wifi-setup | wifi-test | wifi-reset | wifi-scan | wifi-trace | update | help");
}

void showStatus() {
  Serial.printf("Rev K | uptime: %lu s | watchdog: %s | dashboard: %s\n",
                millis() / 1000UL, watchdogEnabled ? "on" : "unavailable",
                updater.active() ? "on" : "off");
  updater.printNetworkStatus();
  for (size_t i = 0; i < 3; ++i) {
    Serial.printf("D%u GPIO%u: PWM %s, duty %u/255\n", unsigned(i + 1),
                  unsigned(ledPins[i]), pwmReady[i] ? "ready" : "failed", unsigned(appliedDuty[i]));
  }
  Serial.printf("Radar OUT: %s | UART RX bytes: %lu\n", presence ? "HIGH" : "LOW",
                (unsigned long)radar.bytesReceived());
  if (lightState == LightState::Ready) {
    Serial.printf("BH1750: %lu.%lu lx%s\n", (unsigned long)(luxTenths / 10),
                  (unsigned long)(luxTenths % 10), sampleFresh ? "" : " (last reading)");
  } else {
    Serial.println(lightState == LightState::Checking ? "BH1750: checking 0x23" :
                                                        "BH1750: unavailable; automatic light off");
  }
}

bool lightAllowsActivation(const LampSettings &settings) {
  return sampleFresh && luxTenths < settings.darknessTenths;
}

String dashboardStatus() {
  const LampSettings &settings = updater.settings();
  const char *light = lightState == LightState::Ready ? "Ready" :
                      lightState == LightState::Checking ? "Checking" : "Sensor unavailable";
  const char *darknessState = "waiting";
  if (sampleFresh) darknessState = lightAllowsActivation(settings) ? "below" : "above";
  else if (lightState == LightState::Unavailable) darknessState = "unavailable";
  char reading[16] = "null";
  if (lightState == LightState::Ready) {
    snprintf(reading, sizeof(reading), "%lu.%lu", (unsigned long)(luxTenths / 10),
             (unsigned long)(luxTenths % 10));
  }
  char leds[3][5];
  for (size_t i = 0; i < 3; ++i) {
    if (pwmReady[i]) snprintf(leds[i], sizeof(leds[i]), "%u", unsigned(appliedDuty[i]));
    else strcpy(leds[i], "null");
  }
  char body[360];
  snprintf(body, sizeof(body),
           "{\"presence\":%s,\"radarBytes\":%lu,\"lux\":%s,\"fresh\":%s,\"light\":\"%s\","
           "\"darknessState\":\"%s\",\"leds\":[%s,%s,%s],"
           "\"settings\":{\"brightness\":%u,\"darkness\":%u.%u,\"holdSeconds\":%u,\"shutoff\":%u.%u},\"radar\":",
           presence ? "true" : "false", (unsigned long)radar.bytesReceived(), reading,
           sampleFresh ? "true" : "false",
           light, darknessState, leds[0], leds[1], leds[2], unsigned(settings.brightness),
           unsigned(settings.darknessTenths / 10), unsigned(settings.darknessTenths % 10),
           unsigned(settings.holdSeconds), unsigned(settings.shutoffTenths / 10), unsigned(settings.shutoffTenths % 10));
  String output(body);
  output += radar.json(millis());
  output += '}';
  return output;
}

void startTest(int first, int last) {
  if (updater.busy()) {
    Serial.println("Firmware update in progress.");
    return;
  }
  testFirst = first;
  testAllOn = false;
  testLast = last;
  reportedChannel = -1;
  testStartedAt = millis();
}

void dashboardTest(int channel, uint8_t brightness) {
  testDuty = (uint16_t(brightness) * 255 + 50) / 100;
  if (channel < 0) {
    testFirst = -1;
    for (size_t i = 0; i < 3; ++i) writeLed(i, 0);
  } else if (channel == 0) {
    startTest(0, 2);
  } else if (channel == 4) {
    startTest(0, 0);
    testAllOn = true;
  } else {
    startTest(channel - 1, channel - 1);
  }
}

void readButton(uint32_t now) {
  const bool pressed = digitalRead(bootButton) == LOW;
  if (pressed && !buttonDown) {
    buttonDownAt = now;
    buttonHandled = false;
  }
  if (!pressed && buttonDown && !buttonHandled && now - buttonDownAt >= wifiResetHoldMs && !updater.busy()) {
    buttonHandled = true;
    updater.resetWifi();
  }
  digitalWrite(blueLed, recoveryLedOn(now - buttonDownAt, pressed, !updater.busy()) ? LOW : HIGH);
  buttonDown = pressed;
}

void handleCommand() {
  if (wifiInput == WifiInput::Ssid) {
    if (commandSize > 32) {
      Serial.println("Wi-Fi setup failed. SSID must fit within 32 bytes.");
      wifiInput = WifiInput::Idle;
      return;
    }
    strlcpy(newSsid, command, sizeof(newSsid));
    wifiInput = WifiInput::Passphrase;
    Serial.println("Wi-Fi passphrase:");
  } else if (wifiInput == WifiInput::Passphrase) {
    wifiInput = WifiInput::Idle;
    if (commandSize < 8 || commandSize > 63) {
      Serial.println("Wi-Fi setup failed. Passphrase must contain 8 to 63 bytes.");
    } else if (updater.configureWifi(newSsid, command, temporaryWifi)) {
      Serial.println(temporaryWifi ? "Wi-Fi test started. Connecting..." : "Wi-Fi saved. Connecting...");
    } else {
      Serial.println("Wi-Fi setup failed. Close firmware-update mode and retry.");
    }
  } else if (strcmp(command, "wifi-setup") == 0 || strcmp(command, "wifi-test") == 0) {
    temporaryWifi = strcmp(command, "wifi-test") == 0;
    wifiInput = WifiInput::Ssid;
    Serial.println("Wi-Fi SSID:");
  } else if (strcmp(command, "test") == 0) startTest(0, 2);
  else if (strncmp(command, "test ", 5) == 0 && strlen(command) == 6 &&
           command[5] >= '1' && command[5] <= '3') {
    const int channel = command[5] - '1';
    startTest(channel, channel);
  } else if (strcmp(command, "status") == 0) showStatus();
  else if (strcmp(command, "wifi-reset") == 0) updater.resetWifi();
  else if (strcmp(command, "wifi-scan") == 0) updater.scanHome();
  else if (strcmp(command, "wifi-trace") == 0) updater.traceHome();
  else if (strcmp(command, "update") == 0) updater.start();
  else if (strcmp(command, "stop") == 0) {
    testFirst = -1;
    updater.stop();
  } else showHelp();
}

void readCommands() {
  for (size_t i = 0; i < sizeof(command) && Serial.available(); ++i) {
    const char ch = Serial.read();
    if (ch == '\r' || ch == '\n') {
      if (commandTooLong) {
        Serial.println("Input too long.");
        wifiInput = WifiInput::Idle;
      }
      else if (commandSize) {
        command[commandSize] = '\0';
        handleCommand();
      }
      commandSize = 0;
      memset(command, 0, sizeof(command));
      commandTooLong = false;
    } else if (commandSize < sizeof(command) - 1) {
      command[commandSize++] = ch;
    } else {
      commandTooLong = true;
    }
  }
}

void driveLamp(uint32_t now) {
  if (updater.busy()) testFirst = -1;
  const uint32_t testElapsed = now - testStartedAt;
  const bool singleChannel = testFirst == testLast;
  const uint32_t step = testStepMs;
  int testChannel = -1;
  if (testFirst >= 0) {
    testChannel = singleChannel ? testFirst : testFirst + testElapsed / step;
    if (testChannel > testLast) {
      testFirst = testChannel = -1;
      Serial.println("LED test complete. Confirm only the indicated LED lit at each step.");
    } else if (testChannel != reportedChannel) {
      reportedChannel = testChannel;
      Serial.printf("Inspect D%u (GPIO%u) now.\n", unsigned(testChannel + 1),
                    unsigned(ledPins[testChannel]));
    }
  }

  const LampSettings &settings = updater.settings();
  const bool suspended = updater.busy() || testChannel >= 0;
  sampleLight(now, !updater.busy());
  const bool dark = lightAllowsActivation(settings);
  const uint8_t normalDuty = lamp.tick(now, presence, dark, suspended, settings,
    sampleFresh && settings.shutoffTenths && luxTenths > settings.shutoffTenths, sampleFresh);
  const uint32_t phase = singleChannel ? testElapsed : testElapsed % step;
  const uint8_t limit = testDuty;
  uint8_t benchDuty = 0;
  if (phase < 250) benchDuty = limit * phase / 250;
  else if (singleChannel || phase < step - 400) benchDuty = limit;
  else benchDuty = uint32_t(limit) * (step - phase) / 400;
  for (size_t i = 0; i < 3; ++i) {
    writeLed(i, testChannel >= 0 ? (testAllOn || testChannel == int(i) ? benchDuty : 0) : normalDuty);
  }
}
}

void setup() {
  digitalWrite(blueLed, HIGH);
  pinMode(blueLed, OUTPUT);
  for (size_t i = 0; i < 3; ++i) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
    pwmReady[i] = ledcAttach(ledPins[i], 4000, 8);
  }
  Serial.begin(115200);
  Serial1.begin(256000, SERIAL_8N1, radarRx, radarTx);
  gpio_pullup_en(gpio_num_t(radarRx));
  pinMode(radarOut, INPUT_PULLDOWN);
  pinMode(bootButton, INPUT_PULLUP);
  Wire.begin(sda, scl, 100000);
  Wire.setTimeOut(25);
  updater.begin(dashboardStatus, dashboardTest);

  const esp_task_wdt_config_t watchdog{15000, 0, true};
  esp_err_t result = esp_task_wdt_init(&watchdog);
  if (result == ESP_ERR_INVALID_STATE) result = esp_task_wdt_reconfigure(&watchdog);
  watchdogEnabled = result == ESP_OK && esp_task_wdt_add(nullptr) == ESP_OK;

  lamp.begin(millis());
  Serial.println("Nightlight Rev K");
  showHelp();
}

void loop() {
  if (watchdogEnabled) esp_task_wdt_reset();
  const uint32_t now = millis();
  readCommands();
  readButton(now);
  readRadar(now);
  driveLamp(millis());
  updater.tick();
  delay(2);
}
