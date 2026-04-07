#include <FastLED.h>

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define NUM_LEDS 64
#define LED_PIN 14

CRGB leds[NUM_LEDS];
uint8_t colorIndex = 0;

// Pre-defined rainbow colors
CRGB rainbowColors[] = {
  CRGB(255, 0, 0),    // Red
  CRGB(255, 127, 0),  // Orange
  CRGB(255, 255, 0),  // Yellow
  CRGB(0, 255, 0),    // Green
  CRGB(0, 0, 255),    // Blue
  CRGB(75, 0, 130),   // Indigo
  CRGB(148, 0, 211)   // Violet
};

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
  Serial.println("RGB Rainbow - Diagonal Flow");
  
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
}

void loop() {
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      // Diagonal distance from top-left corner
      uint8_t diagonal_distance = x + y;
      
      // Map diagonal distance to color index with offset
      uint8_t color_idx = (diagonal_distance + colorIndex) % 7;
      
      leds[XY(x, y)] = rainbowColors[color_idx];
    }
  }
  
  colorIndex++;
  if (colorIndex >= 7) colorIndex = 0;
  
  FastLED.show();
  delay(200);
}