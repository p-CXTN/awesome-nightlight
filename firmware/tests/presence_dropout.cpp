#include "../NightlightRevK/Lamp.h"
#include <cassert>

int main() {
  LampSettings settings;
  settings.brightness = 100;
  Lamp lamp;
  lamp.begin(0);
  uint8_t previous = 0;
  // Repeated dropouts during turn-on must not interrupt the brightness ramp.
  for (uint32_t now = 10000; now <= 16000; now += 10) {
    const bool presence = (now - 10000) % 1000 < 100;
    const uint8_t duty = lamp.tick(now, presence, true, false, settings);
    assert(duty >= previous);
    if (now >= 10500) assert(duty == 255);
    previous = duty;
  }
  assert(lamp.tick(16010, false, true, false, settings) == 255);
  assert(lamp.tick(18009, false, true, false, settings) == 255);
  // A renewed detection restarts the continuous-absence requirement.
  assert(lamp.tick(18010, true, true, false, settings) == 255);
  assert(lamp.tick(18020, false, true, false, settings) == 255);
  assert(lamp.tick(20019, false, true, false, settings) == 255);
  assert(lamp.tick(20020, false, true, false, settings) == 255);
  assert(lamp.tick(21020, false, true, false, settings) > 0);
  assert(lamp.tick(22020, false, true, false, settings) == 0);

  // Other shutoff reasons must bypass the absence grace period.
  for (int cause = 0; cause < 3; ++cause) {
    Lamp other;
    other.begin(0);
    other.tick(10000, true, true, false, settings);
    assert(other.tick(10500, true, true, false, settings) == 255);
    other.tick(10600, false, true, cause == 0, settings, cause == 1, cause != 2);
    assert(other.tick(11600, false, true, cause == 0, settings, cause == 1, cause != 2) < 255);
    assert(other.tick(12600, false, true, cause == 0, settings, cause == 1, cause != 2) == 0);
  }

  Lamp rollover;
  const uint32_t start = UINT32_MAX - 15000;
  rollover.begin(start);
  rollover.tick(start + 10000, true, true, false, settings);
  rollover.tick(start + 10500, true, true, false, settings);
  const uint32_t lostAt = UINT32_MAX - 1000;
  assert(rollover.tick(lostAt, false, true, false, settings) == 255);
  assert(rollover.tick(lostAt + 1999, false, true, false, settings) == 255);
  rollover.tick(lostAt + 2000, false, true, false, settings);
  assert(rollover.tick(lostAt + 4000, false, true, false, settings) == 0);
}
