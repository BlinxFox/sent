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

            // Tick_time can be between 3µs and 90µs 
            // Figure out what divider we need for the 
            // 16 bit timer to allow for at least 4 syncs
            uint32_t cycles = F_CPU / 1000000 * _tick_time * 4;
            uint32_t div = cycles / (UINT16_MAX + 1UL);

            const auto numDivider = 6;
            uint16_t divider[] = {0, 1, 8, 64, 256, 1024};
            int cs1x;
            for(cs1x=1; cs1x<numDivider; cs1x++){
                if(div < divider[cs1x])
                    break;
            }

            if(cs1x >= numDivider){
                onError(SentError::ConfigurationError);
                return;
            }

            auto F_TIMER = F_CPU / divider[cs1x];

            if((F_TIMER / 1000000 * _tick_time) < 4){
                // For this F_CPU and tick_time the divider would not allow
                // for accurate enough measurements with the timer...
                onError(SentError::ConfigurationError);
                return;
            }

            updateLut(F_TIMER);
            TCCR1A = 0;
            TCCR1B = cs1x;
            TCCR1C = 0;
            TIMSK1 = (1 << ICIE1);
        }
};
