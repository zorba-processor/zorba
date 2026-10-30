#!/bin/sh
# Build/test time measurement, runs inside the builder container (cwd /src).
# Usage: measure.sh gcc|clang|warm      Output goes to /src/out
set -u
MODE=$1
OUT=/src/out
mkdir -p "$OUT"
T="$OUT/timings.txt"
: > "$T"
pacman -Sy --noconfirm --needed time >/dev/null 2>&1 || true
if [ -x /usr/bin/time ]; then TV="/usr/bin/time -v -o"; else TV=""; fi

# timed LABEL CMD... : wall seconds into timings.txt, peak RSS into out/LABEL.time
timed() {
  label=$1; shift
  s=$(date +%s.%N)
  if [ -n "$TV" ]; then /usr/bin/time -v -o "$OUT/$label.time" "$@"; else "$@"; fi
  rc=$?
  e=$(date +%s.%N)
  echo "$label wall_s=$(echo "$e - $s" | bc 2>/dev/null || awk "BEGIN{print $e-$s}") rc=$rc" >> "$T"
  return $rc
}

echo "nproc=$(nproc)" >> "$T"
grep -m1 'model name' /proc/cpuinfo >> "$T" 2>/dev/null
free -m >> "$T" 2>/dev/null
gcc --version | head -1 >> "$T"; clang --version | head -1 >> "$T"
B=build/full-release
export CCACHE_SLOPPINESS=pch_defines,time_macros

case $MODE in
gcc)
  export CCACHE_DISABLE=1
  timed configure cmake --preset full-release -DZORBA_USE_PCH=${ZORBA_PCH:-ON}
  timed build sh -c "cmake --build --preset full-release > $OUT/build.log 2>&1"
  echo "warnings=$(grep -c 'warning:' $OUT/build.log) errors=$(grep -c ' error:' $OUT/build.log)" >> "$T"
  cp $B/.ninja_log "$OUT/ninja_log" 2>/dev/null
  ctest --preset full-release -N | tail -3 >> "$T"
  timed test ctest --preset full-release -j"$(nproc)" --output-junit /src/out/junit.xml
  ;;
clang)
  export CCACHE_DISABLE=1 CC=clang CXX=clang++
  timed configure cmake --preset full-release -DZORBA_USE_PCH=${ZORBA_PCH:-ON} -DCMAKE_CXX_FLAGS=-ftime-trace
  timed build cmake --build --preset full-release -- -k 0 || true
  cp $B/.ninja_log "$OUT/ninja_log" 2>/dev/null
  git clone --depth 1 --branch v1.6.0 https://github.com/aras-p/ClangBuildAnalyzer.git /cba-src
  cmake -S /cba-src -B /cba-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ >/dev/null
  cmake --build /cba-build -j"$(nproc)" >/dev/null
  CBA=$(find /cba-build -name ClangBuildAnalyzer -type f | head -1)
  "$CBA" --all $B "$OUT/capture.bin"
  "$CBA" --analyze "$OUT/capture.bin" > "$OUT/cba.txt"
  ;;
warm)
  export CCACHE_DIR=/src/.ccache CCACHE_MAXSIZE=20G
  unset CCACHE_DISABLE
  timed configure cmake --preset full-release -DZORBA_USE_PCH=${ZORBA_PCH:-ON}
  timed build_cold_ccache cmake --build --preset full-release
  ccache -s >> "$T" 2>&1
  H=src/runtime/base/plan_iterator.h
  # (a) touch only: ninja re-runs every dependent TU, ccache hits (content unchanged)
  ninja -C $B -n 2>/dev/null | head -1 >> "$T"
  touch $H
  echo "dependents_after_touch: $(ninja -C $B -n | tail -1)" >> "$T"
  timed rebuild_touch_ccache cmake --build --preset full-release
  # (b) real content change: ccache misses for all dependents
  echo "namespace zorba { inline constexpr int buildtime_marker = 1; } // buildtime" >> $H
  echo "dependents_after_edit: $(ninja -C $B -n | tail -1)" >> "$T"
  timed rebuild_edit cmake --build --preset full-release
  git checkout -- $H
  # (c) no-op build
  timed rebuild_noop cmake --build --preset full-release
  ;;
esac
echo "done mode=$MODE" >> "$T"
