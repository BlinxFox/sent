#pragma once

class Crc4Sent
{
  public:
    Crc4Sent() {}

    void start() {
      state = 0b0101;
    }

    uint8_t finish() {
      return lookup[state];
    }

    void update(uint8_t nibble) {
      state = lookup[state] ^ (nibble & 0xF);
    }

  private:
    uint8_t state = 0;
    const uint8_t lookup[16] = {
      0x0, 0xD, 0x7, 0xA,
      0xE, 0x3, 0x9, 0x4,
      0x1, 0xC, 0x6, 0xB,
      0xF, 0x2, 0x8, 0x5
    };

};

