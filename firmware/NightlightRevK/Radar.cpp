#include "Radar.h"

namespace {
constexpr uint8_t reportHeader[] = {0xf4, 0xf3, 0xf2, 0xf1};
constexpr uint8_t reportFooter[] = {0xf8, 0xf7, 0xf6, 0xf5};
constexpr uint8_t commandHeader[] = {0xfd, 0xfc, 0xfb, 0xfa};
constexpr uint8_t commandFooter[] = {0x04, 0x03, 0x02, 0x01};
constexpr uint16_t enterConfig = 0x00ff;
constexpr uint16_t exitConfig = 0x00fe;
constexpr uint16_t readConfig = 0x0061;
constexpr uint16_t writeRange = 0x0060;
constexpr uint16_t enableEngineering = 0x0062;
constexpr uint16_t disableEngineering = 0x0063;
constexpr uint16_t writeGate = 0x0064;
constexpr uint32_t commandTimeout = 600;
constexpr uint32_t reportTimeout = 1500;

uint16_t word(const uint8_t *data) {
  return uint16_t(data[0]) | (uint16_t(data[1]) << 8);
}

void putWord(uint8_t *data, uint16_t value) {
  data[0] = value & 0xff;
  data[1] = value >> 8;
}

void appendEnergies(String &output, const uint8_t *values, uint8_t count) {
  output += '[';
  for (uint8_t i = 0; i < count; ++i) {
    if (i) output += ',';
    output += String(values[i]);
  }
  output += ']';
}
}

void Radar::tick(uint32_t now) {
  if (used && now - lastByteAt >= 40) used = 0;
  for (size_t i = 0; i < 512 && serial.available(); ++i) {
    const int value = serial.read();
    if (value < 0) break;
    ++bytes;
    lastByteAt = now;
    receive(uint8_t(value), now);
  }
  if (!busy() || now - sentAt < commandTimeout) return;

  if (phase != Phase::Exit) {
    exit(Result::Timeout, now);
  } else if (!retriedExit) {
    retriedExit = true;
    send(Phase::Exit, exitConfig, nullptr, 0, now);
  } else {
    finish(exitResult == Result::Timeout ? Result::Timeout : Result::ExitFailed);
  }
}

bool Radar::readSettings(uint32_t now) {
  if (busy()) return false;
  begin(Operation::Read, now);
  return true;
}

bool Radar::setRange(uint16_t moving, uint16_t stationary, uint16_t absence, uint32_t now) {
  if (busy() || !haveSettings || moving < 2 || moving > settings.maxGate ||
      stationary < 2 || stationary > settings.maxGate) return false;
  requested[0] = moving;
  requested[1] = stationary;
  requested[2] = absence;
  begin(Operation::Range, now);
  return true;
}

bool Radar::setGate(uint16_t gate, uint16_t moving, uint16_t stationary, uint32_t now) {
  if (busy() || !haveSettings || gate > settings.maxGate || moving > 100 || stationary > 100) return false;
  if (gate < 2 && stationary != settings.stationary[gate]) return false;
  requested[0] = gate;
  requested[1] = moving;
  requested[2] = stationary;
  begin(Operation::Gate, now);
  return true;
}

bool Radar::setEnergyReports(bool enabled, uint32_t now) {
  if (busy()) return false;
  begin(enabled ? Operation::EnableEnergy : Operation::DisableEnergy, now);
  return true;
}

void Radar::begin(Operation task, uint32_t now) {
  operation = task;
  result = Result::None;
  const uint8_t mode[] = {1, 0};
  send(Phase::Enter, enterConfig, mode, sizeof(mode), now);
}

void Radar::send(Phase next, uint16_t command, const uint8_t *payload, uint8_t size, uint32_t now) {
  uint8_t packet[30];
  memcpy(packet, commandHeader, sizeof(commandHeader));
  putWord(packet + 4, size + 2);
  putWord(packet + 6, command);
  if (size) memcpy(packet + 8, payload, size);
  memcpy(packet + 8 + size, commandFooter, sizeof(commandFooter));
  phase = next;
  pendingCommand = command;
  sentAt = now;
  serial.write(packet, 12 + size);
}

void Radar::exit(Result finalResult, uint32_t now) {
  exitResult = finalResult;
  retriedExit = false;
  send(Phase::Exit, exitConfig, nullptr, 0, now);
}

void Radar::finish(Result finalResult) {
  phase = Phase::Idle;
  result = finalResult;
  if (operation != Operation::EnableEnergy && operation != Operation::DisableEnergy &&
      result != Result::Read && result != Result::Saved && result != Result::Unchanged) {
    haveSettings = false;
  }
}

void Radar::receive(uint8_t value, uint32_t now) {
  if (used && used < 4) {
    const uint8_t *header = frame[0] == reportHeader[0] ? reportHeader : commandHeader;
    if (value != header[used]) used = 0;
  }
  if (!used && value != reportHeader[0] && value != commandHeader[0]) return;
  frame[used++] = value;
  const bool isReport = frame[0] == reportHeader[0];
  if (used == 6) {
    const uint16_t size = word(frame + 4);
    if (size < (isReport ? 13 : 4) || size > maximumPayload) {
      used = 0;
      return;
    }
    expectedSize = size + 10;
  }
  if (used < 6 || used != expectedSize) return;

  const uint8_t size = expectedSize - 10;
  const uint8_t *footer = isReport ? reportFooter : commandFooter;
  if (!memcmp(frame + 6 + size, footer, 4)) {
    if (isReport) report(frame + 6, size, now);
    else acknowledge(frame + 6, size, now);
  }
  used = 0;
}

void Radar::report(const uint8_t *data, uint8_t size, uint32_t now) {
  if ((data[0] != 1 && data[0] != 2) || (data[0] == 2 && size != 13) ||
      data[1] != 0xaa || data[2] > 6 || data[size - 2] != 0x55 || data[size - 1] != 0) return;
  reading = {data[2], word(data + 3), data[5], word(data + 6), data[8]};
  movingCount = stationaryCount = 0;
  if (data[0] == 1) readGateEnergies(data, size);
  ++frames;
  lastReport = now;
}

void Radar::readGateEnergies(const uint8_t *data, uint8_t size) {
  if (size < 17 || data[11] > highestGate || data[12] > highestGate) return;
  uint8_t moving = data[11] + 1;
  uint8_t stationary = data[12] + 1;
  const uint8_t expected = 15 + moving + stationary;
  if (size != expected && size != expected + 2) {
    // Some revisions keep all nine energies in the report when their range is limited.
    if (size != 33 && size != 35) return;
    moving = stationary = highestGate + 1;
  }
  memcpy(movingEnergy, data + 13, moving);
  memcpy(stationaryEnergy, data + 13 + moving, stationary);
  movingCount = moving;
  stationaryCount = stationary;
}

void Radar::acknowledge(const uint8_t *data, uint8_t size, uint32_t now) {
  if (!busy() || word(data) != (pendingCommand | 0x0100)) return;
  if (phase == Phase::Exit) {
    finish(word(data + 2) == 0 ? exitResult : Result::ExitFailed);
    return;
  }
  if (word(data + 2) != 0) {
    exit(Result::Rejected, now);
    return;
  }
  const bool enable = operation == Operation::EnableEnergy;
  if (phase == Phase::Enter && (enable || operation == Operation::DisableEnergy)) {
    send(Phase::Mode, enable ? enableEngineering : disableEngineering, nullptr, 0, now);
    return;
  }
  if (phase == Phase::Mode) {
    exit(enable ? Result::EnergyOn : Result::EnergyOff, now);
    return;
  }
  if (phase == Phase::Enter || phase == Phase::Write) {
    send(phase == Phase::Enter ? Phase::Read : Phase::Verify, readConfig, nullptr, 0, now);
    return;
  }
  if (!decodeSettings(data, size)) {
    exit(Result::Invalid, now);
    return;
  }
  if (phase == Phase::Verify) {
    exit(matchesRequest() ? Result::Saved : Result::Mismatch, now);
  } else if (operation == Operation::Read) {
    exit(Result::Read, now);
  } else if (matchesRequest()) {
    exit(Result::Unchanged, now);
  } else if ((operation == Operation::Gate &&
              (requested[0] > settings.maxGate || (requested[0] < 2 && requested[2] != settings.stationary[requested[0]]))) ||
             (operation == Operation::Range &&
              (requested[0] > settings.maxGate || requested[1] > settings.maxGate))) {
    exit(Result::Invalid, now);
  } else {
    uint8_t parameters[18]{};
    for (uint8_t i = 0; i < 3; ++i) {
      putWord(parameters + 6 * i, i);
      putWord(parameters + 6 * i + 2, requested[i]);
    }
    send(Phase::Write, operation == Operation::Range ? writeRange : writeGate,
         parameters, sizeof(parameters), now);
  }
}

bool Radar::decodeSettings(const uint8_t *data, uint8_t size) {
  if (size < 8 || data[4] != 0xaa || data[5] < 2 || data[5] > highestGate) return false;
  const uint8_t count = data[5] + 1;
  if (size < 10 + 2 * count || data[6] < 2 || data[6] >= count ||
      data[7] < 2 || data[7] >= count) return false;
  for (uint8_t i = 0; i < 2 * count; ++i) {
    if (data[8 + i] > 100) return false;
  }
  settings.maxGate = data[5];
  settings.movingGate = data[6];
  settings.stationaryGate = data[7];
  memcpy(settings.moving, data + 8, count);
  memcpy(settings.stationary, data + 8 + count, count);
  settings.absence = word(data + 8 + 2 * count);
  haveSettings = true;
  return true;
}

bool Radar::matchesRequest() const {
  if (operation == Operation::Range) {
    return requested[0] == settings.movingGate && requested[1] == settings.stationaryGate &&
           requested[2] == settings.absence;
  }
  return requested[0] <= settings.maxGate && requested[1] == settings.moving[requested[0]] &&
         requested[2] == settings.stationary[requested[0]];
}

String Radar::json(uint32_t now, bool includeSettings) const {
  static constexpr const char *messages[] = {
    "", "Settings read from the radar.", "Saved and verified on the radar.",
    "The radar already has these settings.", "Gate energy reports enabled.", "Gate energy reports disabled.",
    "No reply from the radar. Check power and both UART leads.",
    "The radar rejected the command. Read settings again before retrying.",
    "Unexpected radar settings response. Read settings again.",
    "Read-back did not match. Read the current radar settings before retrying.",
    "The radar did not confirm leaving configuration mode. Read settings again."
  };
  const bool live = frames && now - lastReport < reportTimeout;
  char data[240];
  snprintf(data, sizeof(data),
           "{\"live\":%s,\"presence\":%s,\"frames\":%lu,\"bytes\":%lu,\"target\":%u,\"movingCm\":%u,\"movingEnergy\":%u,"
           "\"stationaryCm\":%u,\"stationaryEnergy\":%u,\"movingGates\":",
           live ? "true" : "false", presence ? "true" : "false", (unsigned long)frames, (unsigned long)bytes,
           unsigned(reading.target),
           unsigned(reading.movingCm), unsigned(reading.movingEnergy), unsigned(reading.stationaryCm),
           unsigned(reading.stationaryEnergy));
  String output(data);
  appendEnergies(output, movingEnergy, live ? movingCount : 0);
  output += ",\"stationaryGates\":";
  appendEnergies(output, stationaryEnergy, live ? stationaryCount : 0);
  if (!includeSettings) {
    output += '}';
    return output;
  }
  snprintf(data, sizeof(data), ",\"busy\":%s,\"error\":%s,\"message\":\"",
           busy() ? "true" : "false", result >= Result::Timeout ? "true" : "false");
  output += data;
  output += messages[static_cast<uint8_t>(result)];
  output += '"';
  output += ",\"settings\":";
  if (!haveSettings || busy()) {
    output += "null}";
    return output;
  }
  snprintf(data, sizeof(data), "{\"maxGate\":%u,\"movingGate\":%u,\"stationaryGate\":%u,\"absence\":%u,\"moving\":[",
           unsigned(settings.maxGate), unsigned(settings.movingGate), unsigned(settings.stationaryGate),
           unsigned(settings.absence));
  output += data;
  for (uint8_t i = 0; i <= settings.maxGate; ++i) {
    if (i) output += ',';
    output += String(settings.moving[i]);
  }
  output += "],\"stationary\":[";
  for (uint8_t i = 0; i <= settings.maxGate; ++i) {
    if (i) output += ',';
    output += String(settings.stationary[i]);
  }
  output += "]}}";
  return output;
}
