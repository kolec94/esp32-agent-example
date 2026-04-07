#include <FastLED.h>

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define NUM_LEDS 64
#define LED_PIN 14

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  Serial.println("Simple Color Test Starting...");
  
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  
  // Fill all LEDs with red
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  
  Serial.println("All LEDs should be RED");
}

void loop() {
  Serial.println("Red...");
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(2000);
  
  Serial.println("Green...");
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(2000);
  
  Serial.println("Blue...");
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(2000);
}