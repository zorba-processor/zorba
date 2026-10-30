#!/bin/sh
# Print the size of the Zorba library built by a preset, as a Markdown table row set.
# Usage: scripts/profile-size.sh <preset>   (run after the build; needs GNU size/strip)
set -eu
preset=$1
dir=build/$preset
echo "### Library size: $preset"
echo
echo "| file | bytes | stripped (--strip-unneeded) | text | data | bss |"
echo "|---|---:|---:|---:|---:|---:|"
for f in $(find "$dir" -name 'libzorba_simplestore*.so*' -type f | sort); do
  tmp=$(mktemp)
  cp "$f" "$tmp"
  strip --strip-unneeded "$tmp"
  set -- $(size "$f" | tail -n 1)
  echo "| $(basename "$f") | $(stat -c %s "$f") | $(stat -c %s "$tmp") | $1 | $2 | $3 |"
  rm -f "$tmp"
done
