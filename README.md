# ESP32-S3 LED Matrix Examples

A collection of Arduino/PlatformIO sketches for the ESP32-S3 8×8 LED matrix
board (Waveshare ESP32-S3-Matrix), covering basic LED tests, animations,
tilt/gyro-controlled games, a WiFi web control panel, and a WiFi-RSSI
visualizer.

## Hardware

- **Board**: ESP32-S3 with an onboard 8×8 WS2812B LED matrix (GPIO 14) and,
  on most units, a QMI8658 IMU (I2C SDA=11, SCL=12) used by the tilt/gyro
  examples.
- **Framework**: Arduino, built via either the Arduino IDE or PlatformIO
  depending on the example (see each folder).

## Repo Structure

```
esp32-agent-example/
├── lib/
│   ├── BoardConfig/    # Single source of truth for pins, dimensions, wiring, color order
│   └── MatrixUtil/     # Shared XY-mapping + serial helpers (MU_XY, MU_ADD_LEDS, etc.)
├── examples/           # Individual sketches - see below
├── tools/
│   └── led_matrix_viz.py   # Terminal visualizer that reads a sketch's serial frame output
└── .claude/skills/esp32-led-matrix/   # Claude Code skill automating build/upload/debug
```

`lib/BoardConfig` + `lib/MatrixUtil` are the shared foundation for the
newer, PlatformIO-based examples (e.g. `RotatingDonut`) - change hardware
config once in `BoardConfig.h` and it applies everywhere. Older/standalone
examples (`Snake`, `tilt-demo`, `LEDWebControl`, etc.) predate this and
manage their own pins/wiring directly in the sketch.

## Examples

| Example | What it does |
|---|---|
| `SimpleTest` | Minimal sanity check that the matrix lights up |
| `BlinkTest` | Basic on/off blink test |
| `SimpleColorTest` | Cycles solid colors across the matrix |
| `SimpleRainbow` / `RGBRainbow` | Basic rainbow color-cycling animations |
| `SpiralRainbow` | Rainbow colors following a spiral pattern |
| `WaveshareRainbow` | Rainbow animation tuned for the Waveshare board's wiring |
| `RainbowAnimation` | Rainbow animation driven through the standalone `WS_Matrix` driver |
| `RainEffect` | Falling "rain drop" particle animation |
| `RotatingDonut` | Spinning 3D ASCII-donut effect rendered to the LED grid (PlatformIO, uses `lib/BoardConfig` + `lib/MatrixUtil`) |
| `Snake` | Classic Snake game controlled by tilting the board (QMI8658 IMU) |
| `tilt-demo` | Minimal demo of reading the IMU to drive on-screen movement |
| `wifi-slam` | Visualizes a chosen WiFi network's signal strength as a color gradient across the matrix |
| `LEDWebControl` | Hosts its own WiFi access point + web control panel: click-to-draw grid, animation presets, speed/brightness sliders, and a gyro-controlled Snake game with live score. See [its README](examples/LEDWebControl/README.md) for setup and usage. |

## Getting Started

1. Pick an example folder under `examples/`.
2. If it has a `platformio.ini`, build/upload with PlatformIO from that
   folder. Otherwise, open the `.ino` in the Arduino IDE (keep any sibling
   `.cpp`/`.h` files in the same folder - the IDE compiles them together).
3. Check the example's own README/header comments for any extra library
   dependencies (e.g. `FastLED`, `SensorLib` for IMU-based examples).

For Claude Code users, `.claude/skills/esp32-led-matrix/SKILL.md` documents
an automated build/upload/debug workflow for this repo.
