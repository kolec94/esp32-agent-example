#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define NUM_LEDS 64
#define LED_PIN 14

// WiFi Access Point credentials
const char* ap_ssid = "ESP32-LED-Matrix";
const char* ap_password = "ledmatrix123";

CRGB leds[NUM_LEDS];
WebServer server(80);

// Animation variables
unsigned long lastAnimationUpdate = 0;
int animationSpeed = 100; // ms between frames
uint8_t animationHue = 0;
int animationStep = 0;
bool animationRunning = false;
String currentAnimation = "none";

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

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 LED Matrix Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    * { box-sizing: border-box; }
    body {
      font-family: Arial, sans-serif;
      background: #1a1a1a;
      color: #fff;
      margin: 0;
      padding: 20px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    h1 { margin-bottom: 10px; }
    .wifi-status {
      background: #333;
      padding: 8px 16px;
      border-radius: 5px;
      font-size: 12px;
      margin-bottom: 20px;
    }
    .matrix-container {
      display: grid;
      grid-template-columns: repeat(8, 50px);
      grid-gap: 4px;
      margin-bottom: 20px;
      padding: 20px;
      background: #2a2a2a;
      border-radius: 10px;
    }
    .led {
      width: 50px;
      height: 50px;
      border: 2px solid #444;
      border-radius: 5px;
      cursor: pointer;
      transition: transform 0.1s;
    }
    .led:hover {
      transform: scale(1.1);
      border-color: #fff;
    }
    .controls {
      background: #2a2a2a;
      padding: 20px;
      border-radius: 10px;
      display: flex;
      flex-direction: column;
      gap: 15px;
      max-width: 450px;
      width: 100%;
    }
    .control-row {
      display: flex;
      align-items: center;
      gap: 10px;
      flex-wrap: wrap;
    }
    label {
      font-weight: bold;
      min-width: 100px;
    }
    input[type="color"] {
      width: 60px;
      height: 40px;
      border: none;
      border-radius: 5px;
      cursor: pointer;
    }
    button {
      padding: 10px 20px;
      background: #4CAF50;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      font-size: 16px;
      transition: background 0.3s;
    }
    button:hover {
      background: #45a049;
    }
    .clear-btn {
      background: #f44336;
    }
    .clear-btn:hover {
      background: #da190b;
    }
    .info {
      margin-top: 10px;
      padding: 10px;
      background: #333;
      border-radius: 5px;
      font-size: 14px;
    }
  </style>
</head>
<body>
  <h1>🌈 ESP32 LED Matrix Control</h1>
  <div class="wifi-status">AP Mode: ESP32-LED-Matrix</div>

  <div class="matrix-container" id="matrix"></div>

  <div class="controls">
    <div class="control-row">
      <label>Color:</label>
      <input type="color" id="colorPicker" value="#ff0000">
    </div>

    <div class="control-row">
      <button onclick="fillAll()">Fill All</button>
      <button class="clear-btn" onclick="clearAll()">Clear All</button>
    </div>

    <div style="border-top: 2px solid #444; margin: 15px 0; padding-top: 15px;">
      <h3 style="margin: 0 0 15px 0; text-align: center; color: #fff;">Animation Presets</h3>
      
      <div class="control-row">
        <button onclick="startAnimation('rainbow')" style="background: linear-gradient(45deg, #ff0000, #ff7f00, #ffff00, #00ff00, #0000ff, #4b0082, #9400d3);">🌈 Rainbow</button>
        <button onclick="startAnimation('spiral')" style="background: #8A2BE2;">🌀 Spiral</button>
      </div>

      <div class="control-row">
        <button onclick="startAnimation('wave')" style="background: #1E90FF;">🌊 Wave</button>
        <button onclick="startAnimation('pulse')" style="background: #FF69B4;">💗 Pulse</button>
      </div>

      <div class="control-row">
        <button onclick="stopAnimation()" class="clear-btn">⏹️ Stop Animation</button>
      </div>
    </div>

    <div class="info">
      Click any LED to set its color, or use animation presets below
    </div>
  </div>

  <script>
    const matrix = document.getElementById('matrix');
    const colorPicker = document.getElementById('colorPicker');

    // Create 8x8 grid
    for (let y = 0; y < 8; y++) {
      for (let x = 0; x < 8; x++) {
        const led = document.createElement('div');
        led.className = 'led';
        led.style.backgroundColor = '#000';
        led.dataset.x = x;
        led.dataset.y = y;
        led.onclick = () => setLED(x, y, colorPicker.value);
        matrix.appendChild(led);
      }
    }

    function hexToRgb(hex) {
      const r = parseInt(hex.slice(1, 3), 16);
      const g = parseInt(hex.slice(3, 5), 16);
      const b = parseInt(hex.slice(5, 7), 16);
      return {r, g, b};
    }

    function setLED(x, y, color) {
      const rgb = hexToRgb(color);
      fetch(`/setled?x=${x}&y=${y}&r=${rgb.r}&g=${rgb.g}&b=${rgb.b}`)
        .then(response => response.text())
        .then(() => {
          const led = document.querySelector(`[data-x="${x}"][data-y="${y}"]`);
          led.style.backgroundColor = color;
        });
    }

    function fillAll() {
      const color = colorPicker.value;
      const rgb = hexToRgb(color);
      fetch(`/fillall?r=${rgb.r}&g=${rgb.g}&b=${rgb.b}`)
        .then(() => {
          document.querySelectorAll('.led').forEach(led => {
            led.style.backgroundColor = color;
          });
        });
    }

    function clearAll() {
      fetch('/clearall')
        .then(() => {
          document.querySelectorAll('.led').forEach(led => {
            led.style.backgroundColor = '#000';
          });
        });
    }

    function startAnimation(type) {
      fetch(`/${type}`)
        .then(response => response.text())
        .then(data => {
          console.log(`Started ${type} animation:`, data);
        })
        .catch(error => {
          console.error('Error starting animation:', error);
        });
    }

    function stopAnimation() {
      fetch('/stop')
        .then(response => response.text())
        .then(data => {
          console.log('Stopped animation:', data);
        })
        .catch(error => {
          console.error('Error stopping animation:', error);
        });
    }
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleSetLED() {
  if (server.hasArg("x") && server.hasArg("y") &&
      server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {

    int x = server.arg("x").toInt();
    int y = server.arg("y").toInt();
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();

    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
      leds[XY(x, y)] = CRGB(r, g, b);
      FastLED.show();
      server.send(200, "text/plain", "OK");
      return;
    }
  }
  server.send(400, "text/plain", "Invalid parameters");
}

void handleFillAll() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();

    fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
    FastLED.show();
    server.send(200, "text/plain", "OK");
    return;
  }
  server.send(400, "text/plain", "Invalid parameters");
}

void handleClearAll() {
  animationRunning = false;
  currentAnimation = "none";
  FastLED.clear();
  FastLED.show();
  server.send(200, "text/plain", "OK");
}

// Animation Functions
void drawRainbow() {
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      uint8_t hue = animationHue + (x * 32) + (y * 16);
      leds[XY(x, y)] = CHSV(hue, 255, 200);
    }
  }
  animationHue += 2;
}

void drawSpiral() {
  FastLED.clear();
  int centerX = MATRIX_WIDTH / 2;
  int centerY = MATRIX_HEIGHT / 2;
  float angle = (animationStep * 0.2) * (PI / 180.0);
  
  for (int r = 0; r < 4; r++) {
    float spiralAngle = angle + (r * PI / 2);
    int x = centerX + (r * cos(spiralAngle));
    int y = centerY + (r * sin(spiralAngle));
    
    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
      uint8_t hue = animationHue + (r * 64);
      leds[XY(x, y)] = CHSV(hue, 255, 255);
    }
  }
  animationStep += 5;
  animationHue += 1;
}

void drawWave() {
  FastLED.clear();
  for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
    float wave = sin((x + animationStep) * 0.5) * 2 + 3;
    int y = constrain(wave, 0, MATRIX_HEIGHT - 1);
    
    uint8_t hue = animationHue + (x * 32);
    leds[XY(x, y)] = CHSV(hue, 255, 255);
    
    // Add trail effect
    if (y > 0) leds[XY(x, y-1)] = CHSV(hue, 255, 100);
    if (y < MATRIX_HEIGHT-1) leds[XY(x, y+1)] = CHSV(hue, 255, 100);
  }
  animationStep += 1;
  animationHue += 2;
}

void drawPulse() {
  FastLED.clear();
  int centerX = MATRIX_WIDTH / 2;
  int centerY = MATRIX_HEIGHT / 2;
  float pulseRadius = (sin(animationStep * 0.1) + 1) * 3;
  
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
      
      if (abs(distance - pulseRadius) < 1.0) {
        uint8_t brightness = 255 - (abs(distance - pulseRadius) * 255);
        leds[XY(x, y)] = CHSV(animationHue, 255, brightness);
      }
    }
  }
  animationStep += 1;
  animationHue += 1;
}

// Animation handlers
void handleRainbow() {
  animationRunning = true;
  currentAnimation = "rainbow";
  animationStep = 0;
  server.send(200, "text/plain", "Rainbow animation started");
}

void handleSpiral() {
  animationRunning = true;
  currentAnimation = "spiral";
  animationStep = 0;
  server.send(200, "text/plain", "Spiral animation started");
}

void handleWave() {
  animationRunning = true;
  currentAnimation = "wave";
  animationStep = 0;
  server.send(200, "text/plain", "Wave animation started");
}

void handlePulse() {
  animationRunning = true;
  currentAnimation = "pulse";
  animationStep = 0;
  server.send(200, "text/plain", "Pulse animation started");
}

void handleStopAnimation() {
  animationRunning = false;
  currentAnimation = "none";
  server.send(200, "text/plain", "Animation stopped");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nESP32 LED Matrix Web Control");
  Serial.println("=============================");

  // Initialize LEDs
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  FastLED.clear();
  FastLED.show();
  Serial.println("LEDs initialized");

  // Create WiFi Access Point
  Serial.println("Creating WiFi Access Point...");

  WiFi.mode(WIFI_AP);
  delay(100);

  bool ap_started = WiFi.softAP(ap_ssid, ap_password);

  if (ap_started) {
    Serial.println("✓ Access Point created successfully!");
    Serial.print("  SSID: ");
    Serial.println(ap_ssid);
    Serial.print("  Password: ");
    Serial.println(ap_password);
    Serial.print("  IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("\n>>> Connect to WiFi and go to http://192.168.4.1 <<<\n");
  } else {
    Serial.println("✗ Failed to create Access Point!");
  }

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/setled", handleSetLED);
  server.on("/fillall", handleFillAll);
  server.on("/clearall", handleClearAll);
  server.on("/rainbow", handleRainbow);
  server.on("/spiral", handleSpiral);
  server.on("/wave", handleWave);
  server.on("/pulse", handlePulse);
  server.on("/stop", handleStopAnimation);

  server.begin();
  Serial.println("Web server started");
  Serial.println("=============================\n");
  
  // Start default animation (rainbow)
  Serial.println("Starting default rainbow animation...");
  animationRunning = true;
  currentAnimation = "rainbow";
  animationStep = 0;
  lastAnimationUpdate = millis();
}

void loop() {
  server.handleClient();
  
  // Handle animations
  if (animationRunning && millis() - lastAnimationUpdate > animationSpeed) {
    lastAnimationUpdate = millis();
    
    if (currentAnimation == "rainbow") {
      drawRainbow();
    } else if (currentAnimation == "spiral") {
      drawSpiral();
    } else if (currentAnimation == "wave") {
      drawWave();
    } else if (currentAnimation == "pulse") {
      drawPulse();
    }
    
    FastLED.show();
  }
}