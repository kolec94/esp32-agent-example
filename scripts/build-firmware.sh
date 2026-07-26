#!/usr/bin/env bash
# Builds a firmware .bin for every example sketch listed below, exports the
# flash parts esp-web-tools needs (bootloader/partitions/boot_app0/app), and
# writes a manifest.json per sketch plus a top-level index.json describing
# build success and program-storage usage for the flasher page to read.
#
# A single sketch failing to compile (or not fitting the partition) does not
# abort the whole run - it's recorded in index.json as unavailable/oversized
# instead, so the flasher page can grey it out with a reason.
set -uo pipefail

# NOTE: assumes an 8MB-flash, PSRAM-equipped ESP32-S3 (Waveshare
# ESP32-S3-Matrix). If your board differs, adjust FQBN accordingly.
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=default,FlashSize=8M,PSRAM=enabled"
OUT_ROOT="firmware-out"

SKETCHES=(
  BlinkTest
  RGBRainbow
  RainEffect
  RainbowAnimation
  SimpleColorTest
  SimpleRainbow
  SimpleTest
  SpiralRainbow
  WaveshareRainbow
  Snake
  tilt-demo
  wifi-slam
  LEDWebControl
)

rm -rf "$OUT_ROOT"
mkdir -p "$OUT_ROOT"

BOOT_APP0=$(find "$HOME/.arduino15/packages/esp32" -name "boot_app0.bin" 2>/dev/null | head -n1 || true)
if [ -z "$BOOT_APP0" ]; then
  echo "WARNING: boot_app0.bin not found in installed esp32 core - manifests will be missing that part"
fi

entries=()

for name in "${SKETCHES[@]}"; do
  dir="examples/$name"
  dest="$OUT_ROOT/$name"
  log="$OUT_ROOT/$name.log"
  mkdir -p "$dest"

  echo "::group::Building $name"
  if arduino-cli compile --fqbn "$FQBN" --output-dir "$dest" "$dir" >"$log" 2>&1; then
    ok=true
  else
    ok=false
    echo "Build FAILED for $name"
  fi
  cat "$log"
  echo "::endgroup::"

  used=0
  max=0
  percent=0

  if $ok; then
    line=$(grep -E "^Sketch uses" "$log" || true)
    used=$(echo "$line" | grep -oE "[0-9]+" | sed -n '1p')
    max=$(echo "$line" | grep -oE "Maximum is [0-9]+" | grep -oE "[0-9]+")
    used=${used:-0}
    max=${max:-0}
    if [ "$max" -gt 0 ]; then
      percent=$(( used * 100 / max ))
    fi

    bootloader=$(find "$dest" -maxdepth 1 -name "*.ino.bootloader.bin" | head -n1 || true)
    partitions=$(find "$dest" -maxdepth 1 -name "*.ino.partitions.bin" | head -n1 || true)
    app=$(find "$dest" -maxdepth 1 -name "*.ino.bin" | head -n1 || true)

    if [ -z "$bootloader" ] || [ -z "$partitions" ] || [ -z "$app" ]; then
      echo "WARNING: expected output files missing for $name - marking unavailable"
      ok=false
    else
      cp "$bootloader" "$dest/bootloader.bin"
      cp "$partitions" "$dest/partitions.bin"
      cp "$app" "$dest/firmware.bin"
      [ -n "$BOOT_APP0" ] && cp "$BOOT_APP0" "$dest/boot_app0.bin"

      # Clean up the arduino-cli-named intermediates so only the
      # standardized files ship to Pages.
      find "$dest" -maxdepth 1 -name "*.ino.*" -delete
      find "$dest" -maxdepth 1 -name "*.d" -delete

      cat > "$dest/manifest.json" <<EOF
{
  "name": "$name",
  "version": "1.0.0",
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        { "path": "bootloader.bin", "offset": 0 },
        { "path": "partitions.bin", "offset": 32768 },
        { "path": "boot_app0.bin", "offset": 57344 },
        { "path": "firmware.bin", "offset": 65536 }
      ]
    }
  ]
}
EOF
    fi
  fi

  if ! $ok; then
    rm -rf "$dest"
  fi

  entries+=("{\"name\":\"$name\",\"ok\":$ok,\"usedBytes\":$used,\"maxBytes\":$max,\"percent\":$percent}")
done

{
  printf '{"generatedAt":"%s","sketches":[' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  ( IFS=,; printf '%s' "${entries[*]}" )
  printf ']}'
} > "$OUT_ROOT/index.json"

echo "=== index.json ==="
cat "$OUT_ROOT/index.json"
