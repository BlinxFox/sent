#pragma once

#include "BaseSent.h"

#include <Arduino.h>

SentBuffer sentBufferT1;

ISR(TIMER1_CAPT_vect) {
  uint16_t value = ICR1;
  sentBufferT1.write(value);
}

class SentTimer1:public BaseSent{
    public:
        SentTimer1(uint8_t tick_time = 3, bool padding = false)
            : BaseSent(tick_time, padding, sentBufferT1)
        {}

        virtual void begin(SentCallback callback) override {
            BaseSent::begin(callback);
            updateLut(F_CPU);
            TCCR1A = 0;
            TCCR1B = 1;
            TCCR1C = 0;
            TIMSK1 = (1 << ICIE1);
        }
};
