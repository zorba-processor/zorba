#!/bin/sh
# Throwaway experiment: does a warm ccache serve stale objects when a header
# inside the precompiled header changes?  Runs in the builder container.
# Env: SLOPPY (CCACHE_SLOPPINESS), EXTRA (extra cmake args), NODIRECT (1/0)
set -eu
OUT=/src/out; mkdir -p $OUT
S=$OUT/summary.txt; : > $S
export CCACHE_DIR=/src/.ccache CCACHE_MAXSIZE=20G CCACHE_SLOPPINESS="$SLOPPY"
[ "${NODIRECT:-0}" = 1 ] && export CCACHE_NODIRECT=1
{ ccache --version | head -1; gcc --version | head -1; cmake --version | head -1
  echo "SLOPPY=$SLOPPY EXTRA=$EXTRA NODIRECT=${NODIRECT:-0}"; } >> $S
Q='declare namespace ver = "http://zorba.io/options/versioning"; declare option ver:zorba-version "5.0"; 1+1'
step() { # name major debug
  name=$1; maj=$2
  sed -i "s/SET(ZORBA_MAJOR_NUMBER \"[0-9]*\")/SET(ZORBA_MAJOR_NUMBER \"$maj\")/" CMakeLists.txt
  cmake --preset embedded-noicu -B build/$name $EXTRA > $OUT/$name.configure.log
  if [ "$3" = 1 ]; then export CCACHE_DEBUG=1 CCACHE_DEBUGDIR=$OUT/dbg-$name; fi
  ccache -z > /dev/null
  s=$(date +%s)
  cmake --build build/$name --target zorbacmd > $OUT/$name.build.log 2>&1
  e=$(date +%s)
  unset CCACHE_DEBUG CCACHE_DEBUGDIR
  { echo "== $name (major=$maj) build_s=$((e - s))"
    grep '#define ZORBA_VERSION ' build/$name/include/zorba/config.h
    ccache -s | grep -E '^ *(Hits|Misses|Direct|Preprocessed|Uncacheable|Errors)' || true
    ccache -s -v | grep -iE 'precompiled|preprocessing failed' || true
    echo "zorba --version: $(build/$name/bin/zorba --version | head -1)"
    echo "query requiring 5.0: $(build/$name/bin/zorba -q "$Q" 2>&1 | tail -1)"; } >> $S
}
step a 3 0     # cold, version 3.1.0
step b 5 1     # warm, version 5.1.0: must rebuild everything that includes config.h
step c 5 0     # warm, same content: measures warm-hit time
cd build/b
ninja -t commands zorbacmd | grep -m1 'cmake_pch.hxx.cxx' > $OUT/pch-create.cmd || true
ninja -t commands zorbacmd | grep -m1 'static_context.cpp' > $OUT/static_context.cmd || true
head -c 600 src/CMakeFiles/zorba_simplestore.dir/cmake_pch.hxx > $OUT/cmake_pch.hxx 2>/dev/null || find . -name cmake_pch.hxx | head -1 >> $OUT/cmake_pch.hxx
cat $S
