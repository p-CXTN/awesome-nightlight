#pragma once

#include <stdint.h>

struct LampSettings {
  static constexpr uint16_t maxDarknessTenths = 10000;
  uint16_t brightness = 25;
  uint16_t darknessTenths = 100;
  uint16_t holdSeconds = 30;

  uint16_t shutoffTenths = 0;  // Zero disables the bright-light override.

  bool valid() const {
    return brightness >= 1 && brightness <= 100 && darknessTenths <= maxDarknessTenths &&
           holdSeconds <= 300 && shutoffTenths <= maxDarknessTenths &&
           (!shutoffTenths || shutoffTenths > darknessTenths);
  }
};

class Lamp {
 public:
  static constexpr uint32_t settlingMs = 10000;
  static constexpr uint32_t fadeOnMs = 500;
  static constexpr uint32_t fadeOffMs = 2000;
  static constexpr uint32_t lightQuietMs = 1000;
  static constexpr uint32_t absenceConfirmMs = 2000;

  void begin(uint32_t now) {
    bootAt = darkSince = now;
  }

  bool canSample(uint32_t now) const {
    return duty == 0 && target == 0 && now - darkSince >= lightQuietMs;
  }

  uint8_t tick(uint32_t now, bool presence, bool dark, bool suspended,
               const LampSettings &settings, bool tooBright = false, bool lightValid = true) {
    const uint8_t previous = duty;
    if (presence || !active) {
      timingAbsence = false;
    } else if (!timingAbsence) {
      timingAbsence = true;
      absenceSince = now;
    }
    const bool absent = timingAbsence && now - absenceSince >= absenceConfirmMs;
    // Finish a fade-out before considering another activation. The lower
    // threshold is only used after the LEDs are fully off and settled.
    if (active && (suspended || absent || !lightValid || tooBright)) {
      active = false;
      fadeTo(0, now);
    }
    if (!active && !suspended && presence && lightValid && dark && !tooBright &&
        now - bootAt >= settlingMs && canSample(now)) {
      active = true;
    }
    if (active) fadeTo((settings.brightness * 255 + 50) / 100, now);

    if (duty != target) {
      const uint32_t elapsed = now - fadeStart;
      duty = elapsed >= fadeDuration
                 ? target
                 : from + (int32_t(target) - from) * int32_t(elapsed) / int32_t(fadeDuration);
    }
    if (duty == 0 && target == 0 && previous != 0) darkSince = now;
    return duty;
  }

 private:
  void fadeTo(uint8_t value, uint32_t now) {
    if (value == target) return;
    from = duty;
    target = value;
    fadeStart = now;
    fadeDuration = value == 0 ? fadeOffMs : fadeOnMs;
  }

  bool active = false;
  bool timingAbsence = false;
  uint32_t absenceSince = 0;
  uint8_t duty = 0;
  uint8_t from = 0;
  uint8_t target = 0;
  uint32_t bootAt = 0;
  uint32_t darkSince = 0;
  uint32_t fadeStart = 0;
  uint32_t fadeDuration = 0;
};
