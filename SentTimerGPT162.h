#pragma once

#include "FspTimer.h"
#include "BaseSent.h"

#include <Arduino.h>

void isr_GPT162(timer_callback_args_t *p_args);

class SentTimerGPT162:public BaseSent{
    public:
        SentTimerGPT162(uint8_t tick_time = 3, bool padding = false)
            : BaseSent(tick_time, padding, sentBuffer)
        {}

        virtual void begin(SentCallback callback) override {
            BaseSent::begin(callback);

            pinMode(4, INPUT);
            R_PFS->PORT[1].PIN[3].PmnPFS_b.PSEL = 0b00011;
            R_PFS->PORT[1].PIN[3].PmnPFS_b.PMR = 0b1;

            timer.force_use_of_pwm_reserved_timer();
            timer.begin(TIMER_MODE_PERIODIC, GPT_TIMER, 2, 0x10000, 0, TIMER_SOURCE_DIV_4, isr_GPT162, this);
            timer.setup_capture_a_irq();
            timer.open();

            updateLut(timer.get_freq_hz());

            R_GPT2->GTICASR_b.ASCAFBL = 0b1;
            R_GPT2->GTICASR_b.ASCARBH = 0b1;
            timer.start();
        }

        void doISR(timer_callback_args_t *p_args){
            if(p_args->event == TIMER_EVENT_CAPTURE_A || p_args->event == TIMER_EVENT_CAPTURE_B){
                sentBuffer.write(p_args->capture);
            }
        }

    private:
        FspTimer timer{};
        SentBuffer sentBuffer;
};

void isr_GPT162(timer_callback_args_t *p_args) {
    SentTimerGPT162 *sentTimer = const_cast<SentTimerGPT162*>(static_cast<const SentTimerGPT162*>(p_args->p_context));
    if(sentTimer){
        sentTimer->doISR(p_args);
    }
}
