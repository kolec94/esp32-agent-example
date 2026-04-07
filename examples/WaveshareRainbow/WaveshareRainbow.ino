#include <FastLED.h>

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define LED_PIN 21
#define BRIGHTNESS 60

CRGB leds[NUM_LEDS];
uint8_t hue_offset = 0;

uint16_t XY(uint8_t x, uint8_t y) {
  if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) return NUM_LEDS;
  
  uint16_t i;
  if (y & 0x01) {
    uint8_t reverseX = (MATRIX_WIDTH - 1) - x;
    i = (y * MATRIX_WIDTH) + reverseX;
  } else {
    i = (y * MATRIX_WIDTH) + x;
  }
  return i;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Waveshare ESP32-S3-Matrix Rainbow Animation");
  
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  
  Serial.println("Rainbow animation starting...");
}

void loop() {
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      // Calculate diagonal distance from top-left corner (0,0) to bottom-right
      uint8_t diagonal_distance = x + y;
      uint8_t hue = hue_offset + (diagonal_distance * 25);
      leds[XY(x, y)] = CHSV(hue, 255, 200);
    }
  }
  
  hue_offset += 3;
  FastLED.show();
  
  delay(80);
}