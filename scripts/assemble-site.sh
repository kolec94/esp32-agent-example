#!/usr/bin/env bash
# Usage: assemble-site.sh <cache-dir> <download-dir> <site-dir>
#
# Builds the final Pages site content under <site-dir>/firmware/, for every
# sketch discover-sketches.sh currently knows about:
#   - if this run freshly built it (found under <download-dir>/firmware-<name>/
#     with a stats.json), use that
#   - else if the previously published site had it (<cache-dir>/firmware/<name>/
#     with a manifest.json), reuse it unchanged
#   - else mark it unavailable (never built, and not built this run)
#
# This is what makes the workflow incremental: sketches nobody touched keep
# their last-known-good binary without being recompiled.
set -uo pipefail

CACHE_DIR="${1:?cache dir required}"
DOWNLOAD_DIR="${2:?download dir required}"
SITE_DIR="${3:?site dir required}"

mkdir -p "$SITE_DIR/firmware"
entries=()

while read -r name; do
  [ -z "$name" ] && continue
  fresh="$DOWNLOAD_DIR/firmware-$name"
  cached="$CACHE_DIR/firmware/$name"
  dest="$SITE_DIR/firmware/$name"
  rm -rf "$dest"

  if [ -f "$fresh/stats.json" ]; then
    echo "$name: using freshly built firmware"
    mkdir -p "$dest"
    cp "$fresh"/manifest.json "$dest"/ 2>/dev/null || true
    cp "$fresh"/*.bin "$dest"/ 2>/dev/null || true
    stats=$(cat "$fresh/stats.json")
  elif [ -f "$cached/manifest.json" ]; then
    echo "$name: reusing previously published firmware (unchanged)"
    mkdir -p "$dest"
    cp "$cached"/manifest.json "$dest"/ 2>/dev/null || true
    cp "$cached"/*.bin "$dest"/ 2>/dev/null || true
    stats=$(jq -c --arg n "$name" '.sketches[] | select(.name == $n)' "$CACHE_DIR/firmware/index.json" 2>/dev/null || true)
    if [ -z "$stats" ]; then
      stats="{\"name\":\"$name\",\"ok\":false,\"usedBytes\":0,\"maxBytes\":0,\"percent\":0}"
    fi
  else
    echo "$name: no build available (never built successfully) - marking unavailable"
    stats="{\"name\":\"$name\",\"ok\":false,\"usedBytes\":0,\"maxBytes\":0,\"percent\":0}"
  fi

  entries+=("$stats")
done < <(scripts/discover-sketches.sh)

{
  printf '{"generatedAt":"%s","sketches":[' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  ( IFS=,; printf '%s' "${entries[*]}" )
  printf ']}'
} > "$SITE_DIR/firmware/index.json"

echo "=== final firmware/index.json ==="
cat "$SITE_DIR/firmware/index.json"
