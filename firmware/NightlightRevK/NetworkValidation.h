#pragma once
#include <stdint.h>

namespace NetworkValidation {
// IPv4 values use network order (the first octet occupies the high byte).
inline bool unicast(uint32_t value) {
  const uint8_t first = value >> 24;
  return first > 0 && first < 224 && first != 127 && (value >> 16) != 0xa9fe;
}
inline bool staticAddress(uint32_t address, uint32_t gateway, uint32_t mask, uint32_t dns) {
  const uint32_t hosts = ~mask;
  if (!unicast(address) || !unicast(gateway) || !unicast(dns) || !mask || hosts < 3 ||
      (hosts & (hosts + 1)) != 0 || (address & mask) != (gateway & mask) || address == gateway ||
      !(address & hosts) || (address & hosts) == hosts ||
      !(gateway & hosts) || (gateway & hosts) == hosts) return false;
  const uint32_t first = address & mask, last = first | hosts;
  return last < 0xc0a80400UL || first > 0xc0a804ffUL;
}
}
