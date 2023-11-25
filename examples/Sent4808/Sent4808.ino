
// Connect data pin of sensor to D5 of the Nano Every clone (ATmega4808)
const uint8_t pin = 5;

#include <SentTimerTCB0.h>

SentTimerTCB0 sent(3, true, pin);

void callback(SentFrame &frame){
    if(Serial1.availableForWrite() > 15){
        for(int c = 0; c < 8; c++){
            Serial1.print(frame[c], HEX);
        }
        Serial1.println();
    }
}

void setup(){
    Serial1.begin(115200);
    sent.begin(&callback);
}

void loop(){
    sent.update();
}
