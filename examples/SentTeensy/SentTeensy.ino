
// Connect data pin of sensor to 18 of the Teensy 4.x

#include <SentTimerTeensyQuad.h>
#include <MLX90372.h>

SentTimerTeensyQuad sent(3, true, 18);
MLX90372 mlx(sent);

unsigned long lastTime;
volatile float mlxPosition;

void mlx_cb(uint16_t sentPos){
  mlxPosition = float(sentPos) / 4096;
}

void setup(){
    Serial.begin(115200);

    sent.begin();
    mlx.begin(&mlx_cb);

    lastTime = millis();
}

void loop(){
    sent.update();

    unsigned long thisTime = millis();
    if(thisTime - lastTime > 100) {
      lastTime = thisTime;

      noInterrupts();
      float localPos = mlxPosition;
      interrupts();

      char buffer[10];
      snprintf(buffer, sizeof(buffer), "%.2f", localPos);
      Serial.println(buffer);
    }
}
