
// Connect data pin of sensor to D49 of the Arduino Mega 2560

#include <SentTimer4.h>

SentTimer4 sent(3, true);

void callback(SentFrame &frame){
    if(Serial.availableForWrite() > 10){
        for(int c = 0; c < 8; c++){
            Serial.print(frame[c], HEX);
        }
        Serial.println();
    }
}

void setup(){
    Serial.begin(115200);
    sent.begin(&callback);
}

void loop(){
    sent.update();
}
