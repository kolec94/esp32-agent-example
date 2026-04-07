#include "WS_Matrix.h"

unsigned long last_update = 0;
const unsigned long UPDATE_INTERVAL = 50;

void setup() {
  Serial.begin(115200);
  Matrix_Init();
  Serial.println("Rainbow Animation Started!");
}

void loop() {
  unsigned long current_time = millis();
  
  if (current_time - last_update >= UPDATE_INTERVAL) {
    RainbowAnimation();
    last_update = current_time;
  }
}