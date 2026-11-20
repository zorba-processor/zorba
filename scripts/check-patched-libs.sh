#!/bin/sh
# Verify, inside a Zorba image, that each given executable loads exactly one
# copy of each of ICU's common, i18n and data libraries and of libxml2, and
# that it is the build from the zorba-processor forks whose checksums the
# image recorded (see docker/*/Dockerfile).
# Usage: check-patched-libs.sh <executable>...
set -u
[ $# -gt 0 ] || { echo "usage: check-patched-libs.sh <executable>..."; exit 2; }
manifest=/usr/local/share/zorba-patched-libs.sha256
fail=0

sha256sum -c "$manifest" || fail=1
for exe in "$@"; do
  deps=$(ldd "$exe") || { echo "FAIL  ldd $exe"; fail=1; continue; }
  for lib in libicuuc libicui18n libicudata libxml2; do
    paths=$(printf '%s\n' "$deps" | awk -v lib="$lib" \
              'index($1, lib ".so") == 1 { print $3 }' | xargs -r readlink -f | sort -u)
    count=$(printf '%s' "$paths" | grep -c .)
    if [ "$count" -ne 1 ]; then
      echo "FAIL  $exe: $count copies of $lib: $(echo $paths)"
      fail=1
    elif ! grep -q "  $paths\$" "$manifest"; then
      echo "FAIL  $exe: $lib is $paths, not the patched build"
      fail=1
    else
      echo "ok    $exe: $lib -> $paths"
    fi
  done
done
exit $fail
