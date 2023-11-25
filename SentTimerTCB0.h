#pragma once

#include "BaseSent.h"

#include <Arduino.h>

SentBuffer sentBufferTCB0;

ISR(TCB0_INT_vect) {
  uint16_t value = TCB0.CCMP;
  sentBufferTCB0.write(value);
}

class SentTimerTCB0:public BaseSent{
    public:
        SentTimerTCB0(uint8_t tick_time = 3, bool padding = false, uint8_t pin = 5)
            : BaseSent(tick_time, padding, sentBufferTCB0)
            , _pin(pin)
        {}

        virtual void begin(SentCallback callback) override {
            BaseSent::begin(callback);
            updateLut(F_CPU);

            uint8_t port0;
            uint8_t port1;
            switch(digitalPinToBitPosition(_pin)){
                case 0: 
                    port0 = EVSYS_GENERATOR_PORT0_PIN0_gc;
                    port1 = EVSYS_GENERATOR_PORT1_PIN0_gc;
                    break;
                case 1: 
                    port0 = EVSYS_GENERATOR_PORT0_PIN1_gc;
                    port1 = EVSYS_GENERATOR_PORT1_PIN1_gc;
                    break;
                case 2: 
                    port0 = EVSYS_GENERATOR_PORT0_PIN2_gc;
                    port1 = EVSYS_GENERATOR_PORT1_PIN2_gc;
                    break;
                case 3: 
                    port0 = EVSYS_GENERATOR_PORT0_PIN3_gc;
                    port1 = EVSYS_GENERATOR_PORT1_PIN3_gc;
                    break;
                case 4: 
                    port0 = EVSYS_GENERATOR_PORT0_PIN4_gc;
                    port1 = EVSYS_GENERATOR_PORT1_PIN4_gc;
                    break;
                case 5: 
                    port0 = EVSYS_GENERATOR_PORT0_PIN5_gc;
                    port1 = EVSYS_GENERATOR_PORT1_PIN5_gc;
                    break;
                case 6: 
                    port0 = EVSYS_GENERATOR_PORT0_PIN6_gc;
                    port1 = EVSYS_GENERATOR_PORT1_PIN6_gc;
                    break;
                case 7: 
                    port0 = EVSYS_GENERATOR_PORT0_PIN7_gc;
                    port1 = EVSYS_GENERATOR_PORT1_PIN7_gc;
                    break;
                default:
                    onError(SentError::ConfigurationError);
                    return;
            }

            uint8_t channel;
            switch(digitalPinToPort(pin)){
                case PA:
                case PB:
                    channel = 0;
                    break;
                case PC:
                case PD:
                    channel = 2;
                    break;
                case PE:
                case PF:
                    channel = 4;
                    break;
                default:
                    onError(SentError::ConfigurationError);
                    return;
            }

            if((channel == 0) && (EVSYS.CHANNEL0 == 0) ){
                EVSYS.CHANNEL0 = port0;
                EVSYS.USERTCB0 = EVSYS_CHANNEL_CHANNEL0_gc;
            } else if((channel == 0) && (EVSYS.CHANNEL1 == 0) ){
                EVSYS.CHANNEL1 = port1;
                EVSYS.USERTCB0 = EVSYS_CHANNEL_CHANNEL1_gc;
            } else if((channel == 2) && (EVSYS.CHANNEL2 == 0) ){
                EVSYS.CHANNEL2 = port0;
                EVSYS.USERTCB0 = EVSYS_CHANNEL_CHANNEL2_gc;
            } else if((channel == 2) && (EVSYS.CHANNEL3 == 0) ){
                EVSYS.CHANNEL3 = port1;
                EVSYS.USERTCB0 = EVSYS_CHANNEL_CHANNEL3_gc;
            } else if((channel == 4) && (EVSYS.CHANNEL4 == 0) ){
                EVSYS.CHANNEL4 = port0;
                EVSYS.USERTCB0 = EVSYS_CHANNEL_CHANNEL4_gc;
            } else if((channel == 4) && (EVSYS.CHANNEL5 == 0) ){
                EVSYS.CHANNEL5 = port1;
                EVSYS.USERTCB0 = EVSYS_CHANNEL_CHANNEL5_gc;
            } else {
                onError(SentError::ConfigurationError);
                return;
            }

            TCB0.CTRLA = 0;
  
            TCB0.CTRLB = 0x02;
            TCB0.EVCTRL = 0x11;
            TCB0.INTCTRL = 1;

            TCB0.CTRLA = 1;
        }
    
    private:
        uint8_t _pin;
};
