
// Connect data pin of sensor to D8 of the Arduino Uno/Nano/Mini

#include <SentTimer1.h>

SentTimer1 sent(3, true);

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
