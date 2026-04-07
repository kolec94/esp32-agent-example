void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 is working!");
  Serial.println("Rainbow test starting...");
}

void loop() {
  Serial.println("Loop running...");
  delay(1000);
}