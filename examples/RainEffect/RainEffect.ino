#include <FastLED.h>

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define NUM_LEDS 64
#define LED_PIN 14
#define MAX_DROPS 12

CRGB leds[NUM_LEDS];

struct RainDrop {
  int8_t x;
  int8_t y;
  uint8_t color_idx;
  uint8_t brightness;
  bool active;
};

RainDrop drops[MAX_DROPS];

// Rain colors (blues, cyans, whites)
CRGB rainColors[] = {
  CRGB(0, 100, 255),   // Light blue
  CRGB(0, 150, 255),   // Bright blue
  CRGB(0, 255, 255),   // Cyan
  CRGB(100, 200, 255), // Sky blue
  CRGB(200, 255, 255), // Light cyan
  CRGB(255, 255, 255)  // White
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

void initializeDrops() {
  for (int i = 0; i < MAX_DROPS; i++) {
    drops[i].active = false;
    drops[i].x = random(0, MATRIX_WIDTH);
    drops[i].y = -1;
    drops[i].color_idx = random(0, 6);
    drops[i].brightness = random(150, 255);
  }
}

void createNewDrop() {
  for (int i = 0; i < MAX_DROPS; i++) {
    if (!drops[i].active) {
      drops[i].active = true;
      drops[i].x = random(0, MATRIX_WIDTH);
      drops[i].y = 0;
      drops[i].color_idx = random(0, 6);
      drops[i].brightness = random(150, 255);
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Rain Effect Animation");
  
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  
  initializeDrops();
  randomSeed(analogRead(0));
}

void loop() {
  // Clear the matrix
  FastLED.clear();
  
  // Randomly create new drops
  if (random(0, 100) < 30) {  // 30% chance each frame
    createNewDrop();
  }
  
  // Update and draw all drops
  for (int i = 0; i < MAX_DROPS; i++) {
    if (drops[i].active) {
      // Draw the drop
      if (drops[i].y >= 0 && drops[i].y < MATRIX_HEIGHT) {
        CRGB color = rainColors[drops[i].color_idx];
        color.nscale8(drops[i].brightness);
        leds[XY(drops[i].x, drops[i].y)] = color;
        
        // Add trail effect
        if (drops[i].y > 0) {
          CRGB trailColor = rainColors[drops[i].color_idx];
          trailColor.nscale8(drops[i].brightness / 3);
          leds[XY(drops[i].x, drops[i].y - 1)] += trailColor;
        }
        if (drops[i].y > 1) {
          CRGB trailColor = rainColors[drops[i].color_idx];
          trailColor.nscale8(drops[i].brightness / 6);
          leds[XY(drops[i].x, drops[i].y - 2)] += trailColor;
        }
      }
      
      // Move drop down
      drops[i].y++;
      
      // Remove drop if it's off screen
      if (drops[i].y >= MATRIX_HEIGHT + 2) {
        drops[i].active = false;
      }
    }
  }
  
  FastLED.show();
  delay(120);
}