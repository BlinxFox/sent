
#include "crc4.hpp"

#include <avr/pgmspace.h>

static const uint8_t PROGMEM crc4lookup[16] = {
  0x0, 0xD, 0x7, 0xA,
  0xE, 0x3, 0x9, 0x4,
  0x1, 0xC, 0x6, 0xB,
  0xF, 0x2, 0x8, 0x5
};

Crc4Sent::Crc4Sent() {
}

void Crc4Sent::start() {
  state = 0b0101;
}

uint8_t Crc4Sent::finish() {
  return pgm_read_byte(&crc4lookup[state]);
}

void Crc4Sent::update4(uint8_t nibble) {
  state = pgm_read_byte(&crc4lookup[state]) ^ (nibble & 0xF);
}

