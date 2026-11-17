#pragma once
#include <stdint.h>

inline bool recoveryLedOn(uint32_t heldMs, bool pressed, bool available) {
  if (!pressed || !available || heldMs < 10000) return false;
  return ((heldMs - 10000) / 100) % 2 == 0;
}
