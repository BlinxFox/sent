#pragma once

#include "BaseSent.h"

#include <Arduino.h>

SentBuffer sentBufferT4;

ISR(TIMER4_CAPT_vect) {
  uint16_t value = ICR4;
  sentBufferT4.write(value);
}

class SentTimer4:public BaseSent{
    public:
        SentTimer4(uint8_t tick_time = 3, bool padding = false)
            : BaseSent(tick_time, padding, sentBufferT4)
        {}

        virtual void begin(SentCallback callback) override {
            BaseSent::begin(callback);
            updateLut(F_CPU);
            TCCR4A = 0;
            TCCR4B = 1;
            TCCR4C = 0;
            TIMSK4 = (1 << ICIE4);
        }
};
