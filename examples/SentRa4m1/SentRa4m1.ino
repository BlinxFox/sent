
// Connect data pin of sensor to D4 (P103) of the Uno R4 minima

#include <SentTimerGPT162.h>

SentTimerGPT162 sent(3, true);

void callback(SentFrame &frame){
    if(Serial.availableForWrite() > 15){
        for(int c = 0; c < 8; c++){
            Serial.print(frame[c], HEX);
        }
        Serial.println();
    }
}

void setup(){
    Serial.begin(115200);
    while(!Serial) {}

    sent.begin(&callback);
}

void loop(){
    sent.update();
}
