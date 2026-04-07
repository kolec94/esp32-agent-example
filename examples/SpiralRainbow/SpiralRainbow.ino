#include <FastLED.h>

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define NUM_LEDS 64
#define LED_PIN 14

CRGB leds[NUM_LEDS];
uint8_t colorOffset = 0;

// Pre-defined rainbow colors
CRGB rainbowColors[] = {
  CRGB(255, 0, 0),    // Red
  CRGB(255, 127, 0),  // Orange
  CRGB(255, 255, 0),  // Yellow
  CRGB(0, 255, 0),    // Green
  CRGB(0, 255, 255),  // Cyan
  CRGB(0, 0, 255),    // Blue
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

float calculateAngle(int8_t x, int8_t y) {
  // Calculate angle from center of matrix
  float centerX = 3.5;
  float centerY = 3.5;
  float dx = x - centerX;
  float dy = y - centerY;
  
  float angle = atan2(dy, dx);
  if (angle < 0) angle += 2 * PI;
  
  return angle;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Spiral Rainbow Animation");
  
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
}

void loop() {
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      // Calculate angle from center and distance
      float angle = calculateAngle(x, y);
      float distance = sqrt(pow(x - 3.5, 2) + pow(y - 3.5, 2));
      
      // Create spiral effect by combining angle and distance
      float spiral_value = (angle * 1.5) + (distance * 0.8) + (colorOffset * 0.1);
      
      // Map to color index
      uint8_t color_idx = ((int)(spiral_value * 10) + colorOffset) % 7;
      
      leds[XY(x, y)] = rainbowColors[color_idx];
    }
  }
  
  colorOffset += 3;
  FastLED.show();
  delay(100);
}