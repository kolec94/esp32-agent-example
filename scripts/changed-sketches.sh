#!/usr/bin/env bash
# Usage: changed-sketches.sh <base-ref> <head-ref>
#
# Prints a JSON array of sketch names (from discover-sketches.sh) whose
# examples/<name>/ path changed between base and head. If shared code
# changed instead (lib/, or the per-sketch build script itself), every
# discovered sketch is returned, since that affects all of them.
#
# If <base-ref> doesn't resolve (e.g. first commit, or a force-push that
# rewrote history), everything is returned rather than guessing.
set -uo pipefail

base="$1"
head="$2"

all_json=$(scripts/discover-sketches.sh | jq -R -s -c 'split("\n") | map(select(length>0))')

if ! git rev-parse --verify "${base}^{commit}" >/dev/null 2>&1; then
  echo "$all_json"
  exit 0
fi

diff_files=$(git diff --name-only "$base" "$head" -- examples/ lib/ scripts/build-firmware.sh)

if echo "$diff_files" | grep -qE '^(lib/|scripts/build-firmware\.sh)'; then
  echo "$all_json"
  exit 0
fi

changed_json=$(echo "$diff_files" \
  | grep -oE '^examples/[^/]+' \
  | sed 's#^examples/##' \
  | sort -u \
  | jq -R -s -c 'split("\n") | map(select(length>0))')

# Intersect with the discovered/buildable list, so a change to a
# .flasher-skip'd example (or one that no longer exists) doesn't try to
# build something that isn't there.
jq -n --argjson all "$all_json" --argjson changed "$changed_json" '
  $all | map(select(. as $n | $changed | index($n) != null))
'
