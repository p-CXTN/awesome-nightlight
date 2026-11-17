#pragma once

#include <Arduino.h>

class Radar {
 public:
  explicit Radar(HardwareSerial &serial) : serial(serial) {}

  void tick(uint32_t now);
  bool readSettings(uint32_t now);
  bool setRange(uint16_t moving, uint16_t stationary, uint16_t absence, uint32_t now);
  bool setGate(uint16_t gate, uint16_t moving, uint16_t stationary, uint32_t now);
  bool setEnergyReports(bool enabled, uint32_t now);
  void setPresence(bool detected) { presence = detected; }
  bool busy() const { return phase != Phase::Idle; }
  uint32_t bytesReceived() const { return bytes; }
  String json(uint32_t now, bool includeSettings = true) const;

 private:
  static constexpr uint8_t highestGate = 8;
  static constexpr uint8_t maximumPayload = 64;

  struct Reading {
    uint8_t target = 0;
    uint16_t movingCm = 0;
    uint8_t movingEnergy = 0;
    uint16_t stationaryCm = 0;
    uint8_t stationaryEnergy = 0;
  };

  struct Settings {
    uint8_t maxGate = 0;
    uint8_t movingGate = 0;
    uint8_t stationaryGate = 0;
    uint16_t absence = 0;
    uint8_t moving[highestGate + 1]{};
    uint8_t stationary[highestGate + 1]{};
  };

  enum class Operation : uint8_t { Read, Range, Gate, EnableEnergy, DisableEnergy };
  enum class Phase : uint8_t { Idle, Enter, Read, Write, Verify, Mode, Exit };
  enum class Result : uint8_t { None, Read, Saved, Unchanged, EnergyOn, EnergyOff, Timeout, Rejected, Invalid, Mismatch, ExitFailed };

  void begin(Operation task, uint32_t now);
  void send(Phase next, uint16_t command, const uint8_t *payload, uint8_t size, uint32_t now);
  void exit(Result result, uint32_t now);
  void finish(Result result);
  void receive(uint8_t value, uint32_t now);
  void report(const uint8_t *payload, uint8_t size, uint32_t now);
  void readGateEnergies(const uint8_t *payload, uint8_t size);
  void acknowledge(const uint8_t *payload, uint8_t size, uint32_t now);
  bool decodeSettings(const uint8_t *payload, uint8_t size);
  bool matchesRequest() const;

  HardwareSerial &serial;
  uint8_t frame[maximumPayload + 10]{};
  uint8_t used = 0;
  uint8_t expectedSize = 0;
  uint32_t lastByteAt = 0;
  uint32_t bytes = 0;
  uint32_t frames = 0;
  uint32_t lastReport = 0;
  bool presence = false;
  Reading reading;
  uint8_t movingEnergy[highestGate + 1]{};
  uint8_t stationaryEnergy[highestGate + 1]{};
  uint8_t movingCount = 0;
  uint8_t stationaryCount = 0;
  Settings settings;
  bool haveSettings = false;
  Operation operation = Operation::Read;
  Phase phase = Phase::Idle;
  Result result = Result::None;
  Result exitResult = Result::None;
  bool retriedExit = false;
  uint16_t pendingCommand = 0;
  uint16_t requested[3]{};
  uint32_t sentAt = 0;
};
