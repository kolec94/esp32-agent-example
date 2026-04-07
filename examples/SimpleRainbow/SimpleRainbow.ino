#include <FastLED.h>

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define NUM_LEDS 64
#define LED_PIN 21

CRGB leds[NUM_LEDS];
uint8_t hue = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Simple Rainbow Test");
  
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
}

void loop() {
  // Fill all LEDs with the same hue, then increment
  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255));
  FastLED.show();
  
  hue += 2;
  delay(50);
}