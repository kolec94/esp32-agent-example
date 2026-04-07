void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);  // Built-in LED on most ESP32s
  Serial.println("ESP32 Blink Test Starting...");
}

void loop() {
  digitalWrite(2, HIGH);
  Serial.println("LED ON");
  delay(1000);
  
  digitalWrite(2, LOW);
  Serial.println("LED OFF");
  delay(1000);
}