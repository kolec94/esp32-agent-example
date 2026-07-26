#!/usr/bin/env bash
# Usage: build-firmware.sh <sketch-name>
#
# Builds ONE example sketch (a directory name under examples/), exports the
# flash parts esp-web-tools needs (bootloader/partitions/boot_app0/app), and
# writes a manifest.json plus a stats.json (build success + program-storage
# usage) into firmware-out/<sketch-name>/.
#
# A failed build (or missing expected output) is not treated as fatal to the
# overall CI run - it's recorded in stats.json as ok:false instead, so the
# flasher page can grey that sketch out with a reason.
set -uo pipefail

name="$1"
dir="examples/$name"
OUT_ROOT="firmware-out"
dest="$OUT_ROOT/$name"

# NOTE: assumes a 4MB-flash, PSRAM-equipped ESP32-S3 (Waveshare
# ESP32-S3-Matrix). Confirmed via hardware: a build with FlashSize=8M
# produced a firmware image header mismatch against the real 4MB chip,
# causing a boot-time assert loop (spi_flash: Detected size(4096k)
# smaller than the size in the binary image header(8192k)). If your
# board differs, adjust FQBN accordingly.
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=default,FlashSize=4M,PSRAM=enabled"

rm -rf "$dest"
mkdir -p "$dest"
log="$OUT_ROOT/$name.log"

BOOT_APP0=$(find "$HOME/.arduino15/packages/esp32" -name "boot_app0.bin" 2>/dev/null | head -n1 || true)
if [ -z "$BOOT_APP0" ]; then
  echo "WARNING: boot_app0.bin not found in installed esp32 core"
fi

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

    # Drop the arduino-cli-named intermediates; only the standardized
    # files above should ship to Pages.
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
  mkdir -p "$dest"
fi

cat > "$dest/stats.json" <<EOF
{"name":"$name","ok":$ok,"usedBytes":$used,"maxBytes":$max,"percent":$percent}
EOF

echo "=== stats.json ==="
cat "$dest/stats.json"
