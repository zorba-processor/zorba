#!/bin/bash -eu
# Build Zorba's libFuzzer targets for ClusterFuzzLite / OSS-Fuzz.
# Inputs from the environment: CC, CXX, CFLAGS, CXXFLAGS, LIB_FUZZING_ENGINE,
# SRC, OUT, SANITIZER.

FUZZ_DIR="$SRC/zorba/test/fuzz"
BUILD="$WORK/zorba-build"
mkdir -p "$BUILD"

# CMAKE_BUILD_TYPE=None: use the flags OSS-Fuzz provides, plus NDEBUG like a
# release build. NDEBUG matters beyond assert(): without it the translator
# loads the source of Zorba's builtin modules on import, and those files are
# not shipped to the fuzzing runner. ZORBA_ASSERT stays active regardless.
export CFLAGS="$CFLAGS -DNDEBUG"
export CXXFLAGS="$CXXFLAGS -DNDEBUG"
cmake -S "$SRC/zorba" -B "$BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=None \
  -DZORBA_BUILD_FUZZERS=ON \
  -DZORBA_NO_XMLSCHEMA=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_XercesC=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_CURL=ON \
  -DZORBA_WITH_UNIT_TESTS=OFF \
  -DZORBA_SUPPRESS_SWIG=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_Java=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_JNI=ON
cmake --build "$BUILD" --target fuzzers

TARGETS="xquery_compile jsoniq_compile xml_parse json_parse"

# libzorba is a shared library: ship it and every non-glibc dependency in
# $OUT/lib (the runner image has none of them) and point the rpaths there.
mkdir -p "$OUT/lib"
for t in $TARGETS; do
  cp "$BUILD/test/fuzz/$t" "$OUT/"
done
for t in $TARGETS; do
  ldd "$OUT/$t" | awk '/=> \// { print $3 }' | while read -r lib; do
    case "$(basename "$lib")" in
      libc.so*|libm.so*|libdl.so*|libpthread.so*|librt.so*|ld-linux*|libgcc_s.so*) ;;
      *) cp -L -n "$lib" "$OUT/lib/" ;;
    esac
  done
done
for t in $TARGETS; do
  patchelf --force-rpath --set-rpath '$ORIGIN/lib' "$OUT/$t"
done
for lib in "$OUT"/lib/*.so*; do
  patchelf --force-rpath --set-rpath '$ORIGIN' "$lib"
done

# Dictionaries and options (picked up by name: <target>.dict, <target>.options).
cp "$FUZZ_DIR/xquery.dict" "$OUT/xquery_compile.dict"
cp "$FUZZ_DIR/xquery.dict" "$OUT/jsoniq_compile.dict"
cp "$FUZZ_DIR/xml.dict" "$OUT/xml_parse.dict"
cp "$FUZZ_DIR/json.dict" "$OUT/json_parse.dict"
# Leak detection is off for now (both libFuzzer's per-input check and
# LeakSanitizer at exit): leaks would stop every run before it reaches
# memory-safety bugs.
for t in $TARGETS; do
  case $t in
    *_compile) max_len=4096 ;;
    *) max_len=16384 ;;
  esac
  printf '[libfuzzer]\nmax_len = %s\ndetect_leaks = 0\n\n[asan]\ndetect_leaks = 0\n' \
    "$max_len" > "$OUT/$t.options"
done

python3 "$FUZZ_DIR/make_seed_corpora.py" "$SRC/zorba" "$OUT"
