#include "WS_Matrix.h"
#include <Arduino.h>

Adafruit_NeoPixel pixels(RGB_COUNT, RGB_Control_PIN, NEO_RGB + NEO_KHZ800);

uint8_t hue_offset = 0;

void Matrix_Init() {
  pixels.begin();
  pixels.setBrightness(50);
  pixels.clear();
  pixels.show();
}

uint32_t HSVtoRGB(uint8_t hue, uint8_t saturation, uint8_t brightness) {
  uint8_t r, g, b;
  uint8_t region, remainder, p, q, t;

  if (saturation == 0) {
    r = g = b = brightness;
    return pixels.Color(r, g, b);
  }

  region = hue / 43;
  remainder = (hue - (region * 43)) * 6; 

  p = (brightness * (255 - saturation)) >> 8;
  q = (brightness * (255 - ((saturation * remainder) >> 8))) >> 8;
  t = (brightness * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
    case 0:
      r = brightness; g = t; b = p;
      break;
    case 1:
      r = q; g = brightness; b = p;
      break;
    case 2:
      r = p; g = brightness; b = t;
      break;
    case 3:
      r = p; g = q; b = brightness;
      break;
    case 4:
      r = t; g = p; b = brightness;
      break;
    default:
      r = brightness; g = p; b = q;
      break;
  }

  return pixels.Color(r, g, b);
}

void RainbowAnimation() {
  for (uint8_t y = 0; y < Matrix_Row; y++) {
    for (uint8_t x = 0; x < Matrix_Col; x++) {
      uint8_t hue = hue_offset + (x * 32) + (y * 16);
      uint32_t color = HSVtoRGB(hue, 255, 150);
      
      uint8_t led_index = y * Matrix_Col + x;
      pixels.setPixelColor(led_index, color);
    }
  }
  
  hue_offset += 2;
  pixels.show();
}