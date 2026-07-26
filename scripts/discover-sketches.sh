#!/usr/bin/env bash
# Lists every examples/<name> directory that looks like a standalone Arduino
# sketch (a .ino file directly inside it) and isn't explicitly marked
# unsupported via a ".flasher-skip" file (e.g. PlatformIO-only examples that
# rely on lib_extra_dirs / parent-relative includes arduino-cli can't
# resolve the same way).
set -uo pipefail

for dir in examples/*/; do
  name=$(basename "$dir")
  [ -f "${dir}.flasher-skip" ] && continue
  if compgen -G "${dir}"*.ino > /dev/null; then
    echo "$name"
  fi
done | sort
