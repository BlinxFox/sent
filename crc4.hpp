#pragma once

#include <Arduino.h>

class Crc4Sent
{
  public:
    Crc4Sent();
    void start();
    uint8_t finish();
    void update4(uint8_t nibble);
  private:
    uint8_t state = 0;
};

