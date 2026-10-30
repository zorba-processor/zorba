#!/bin/sh
# Compiles and runs the C++ embedding examples against the Zorba installed in
# the runtime image (/opt/zorba). Installs g++ first.
#
# Usage (inside the Zorba runtime image, as root):
#   run_cpp_examples.sh <examples-dir> <output-dir>
set -u
src=$(cd "$1" && pwd)
out=$2
mkdir -p "$out"
apt-get update -qq >/dev/null && apt-get install -y -qq g++ >/dev/null || exit 1
cd "$src"
find . -type f -name '*.cpp' | sort | while read -r f; do
  f=${f#./}
  base=${f%.*}
  mkdir -p "$out/$(dirname "$f")"
  if g++ -std=c++23 -I/opt/zorba/include "$src/$f" -L/opt/zorba/lib \
       -lzorba_simplestore -o /tmp/example 2> "$out/$base.err"; then
    /tmp/example > "$out/$base.out" 2>> "$out/$base.err"
    echo $? > "$out/$base.rc"
  else
    : > "$out/$base.out"
    echo 99 > "$out/$base.rc"
  fi
  echo "ran $f (exit $(cat "$out/$base.rc"))"
done
