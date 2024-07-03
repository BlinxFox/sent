#pragma once

#include "sent/BaseSent.h"
#include "imxrt.h"

const unsigned char numPins = 16;

const unsigned char validPins[numPins] = {
    10, 12,  1, 13, 40, 41, 19, 42, 18, 43, 14, 44, 15, 45, 6,  9,
};

const unsigned char tmrIndex[numPins] = {
    1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4,
}; // These match the datasheet numbering and get decremented when used

const unsigned char chnIndex[numPins] = {
    0, 1, 2, 0, 1, 2, 0, 0, 1, 1, 2, 2, 3, 0, 1, 2,
};

volatile uint32_t *ioMuxCtrl[numPins] = {
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_01,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_02,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_04,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_05,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_00,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_06,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_01,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_07,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_08,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_03,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_09,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_10,
    &IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11,
};

unsigned char ioMuxValue[numPins] {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

volatile uint32_t *ioSelCtrl[numPins] = {
    nullptr,
    nullptr,
    nullptr,
    &IOMUXC_QTIMER2_TIMER0_SELECT_INPUT,
    &IOMUXC_QTIMER2_TIMER1_SELECT_INPUT,
    &IOMUXC_QTIMER2_TIMER2_SELECT_INPUT,
    &IOMUXC_QTIMER3_TIMER0_SELECT_INPUT,
    &IOMUXC_QTIMER3_TIMER0_SELECT_INPUT,
    &IOMUXC_QTIMER3_TIMER1_SELECT_INPUT,
    &IOMUXC_QTIMER3_TIMER1_SELECT_INPUT,
    &IOMUXC_QTIMER3_TIMER2_SELECT_INPUT,
    &IOMUXC_QTIMER3_TIMER2_SELECT_INPUT,
    &IOMUXC_QTIMER3_TIMER3_SELECT_INPUT,
    nullptr,
    nullptr,
    nullptr,
};

unsigned char ioSelValue[numPins] {
    0, 0, 0, 1, 1, 1, 1, 2, 0, 2, 1, 2, 1, 0, 0, 0,
};

const int numTimers = 4;
const int numTimerChannels = 4;

volatile IMXRT_TMR_t *quadTimers[numTimers] = {
    (IMXRT_TMR_t *)IMXRT_TMR1_ADDRESS, 
    (IMXRT_TMR_t *)IMXRT_TMR2_ADDRESS, 
    (IMXRT_TMR_t *)IMXRT_TMR3_ADDRESS, 
    (IMXRT_TMR_t *)IMXRT_TMR4_ADDRESS
};

void isr_tmr1();
void isr_tmr2();
void isr_tmr3();
void isr_tmr4();

void (*isrFuncs[numTimers])() = {isr_tmr1, isr_tmr2, isr_tmr3, isr_tmr4};
IRQ_NUMBER_t irqNumbers[numTimers] = {IRQ_QTIMER1, IRQ_QTIMER2, IRQ_QTIMER3, IRQ_QTIMER4};
void *listeners[numTimers][numTimerChannels] = {nullptr};

class SentTimerTeensyQuad:public BaseSent{
    public:
        SentTimerTeensyQuad(uint8_t tick_time = 3, bool padding = false, unsigned int pin = 4)
            : BaseSent(tick_time, padding, sentBuffer)
            , _pin{pin}
        {}

        void begin(SentCallback callback = nullptr) override {
            bool isValid = false;
            unsigned char pinIndex;
            for(pinIndex=0; pinIndex<sizeof(validPins); pinIndex++) {
                if(_pin == validPins[pinIndex]) {
                    quadTimer = quadTimers[tmrIndex[pinIndex]-1];
                    quadTimerCh = &(quadTimer->CH[chnIndex[pinIndex]]);
                    isValid = true;
                    break;
                }
            }

            if(!isValid) {
                return;
            }

            pinMode(_pin, INPUT);
            *ioMuxCtrl[pinIndex] = ioMuxValue[pinIndex];
            if(ioSelCtrl[pinIndex] != nullptr) *ioSelCtrl[pinIndex] = ioSelValue[pinIndex];

            attachInterruptVector(irqNumbers[tmrIndex[pinIndex]-1], isrFuncs[tmrIndex[pinIndex]-1]);
            listeners[tmrIndex[pinIndex]-1][chnIndex[pinIndex]] = this;
            NVIC_ENABLE_IRQ(irqNumbers[tmrIndex[pinIndex]-1]);

            quadTimerCh->CTRL = TMR_CTRL_CM(0b001) | TMR_CTRL_PCS(0b1010) | TMR_CTRL_SCS(chnIndex[pinIndex]);
            quadTimerCh->SCTRL = TMR_SCTRL_IEFIE | TMR_SCTRL_CAPTURE_MODE(2);
            quadTimer->ENBL |= 1 << chnIndex[pinIndex];

            updateLut(F_BUS_ACTUAL/4L); //4 comes from TMR_CTRL_PCS above

            BaseSent::begin(callback);
        }

        void doISR(){
            if(quadTimerCh->SCTRL & TMR_SCTRL_IEF) {
                sentBuffer.write(quadTimerCh->CAPT);
                quadTimerCh->SCTRL &= ~(TMR_SCTRL_IEF);
            }
        }

    private:
        SentBuffer sentBuffer;
        volatile IMXRT_TMR_t *quadTimer;
        volatile IMXRT_TMR_CH_t *quadTimerCh;
        unsigned int _pin;
};

void isr_tmr(uint8_t channel) {
    for(uint8_t i=0; i<numTimerChannels; i++) {
        SentTimerTeensyQuad *sentTimer = const_cast<SentTimerTeensyQuad*>(static_cast<const SentTimerTeensyQuad*>(listeners[channel][i]));
        if(sentTimer != nullptr) {
            sentTimer->doISR();
        }
    }
}

void isr_tmr1() {
    isr_tmr(0);
    return;
}

void isr_tmr2() {
    isr_tmr(1);
    return;
}

void isr_tmr3() {
    isr_tmr(2);
    return;
}

void isr_tmr4() {
    isr_tmr(3);
    return;
}