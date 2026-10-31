#!/bin/sh
# Build the source tree DIR in the builder image and run the benchmark suite on it.
# Usage: bench-ref.sh DIR LABEL OUTNAME   (run from the workspace root; needs
# zorba-builder:ci, bench-data/, .ccache/ and out/). Writes out/OUTNAME.{json,md}
# and appends the Markdown to $GITHUB_STEP_SUMMARY when set.
set -eu
DIR=$1 LABEL=$2 NAME=$3
mkdir -p out .ccache

docker run --rm \
  -e CCACHE_DIR=/ccache -e CCACHE_SLOPPINESS=pch_defines,time_macros -e CCACHE_DEPEND=1 \
  -e LABEL="$LABEL" -e RUNS="${RUNS:-5}" -e QUERY_TIMEOUT="${QUERY_TIMEOUT:-300}" -e NAME="$NAME" \
  -v "$PWD/$DIR":/src -v "$PWD/.ccache":/ccache \
  -v "$PWD/bench":/bench:ro -v "$PWD/bench-data":/data:ro -v "$PWD/out":/out \
  -w /src zorba-builder:ci sh -ec '
    cmake --preset full-release
    cmake --build --preset full-release
    ccache -s
    ZORBA=$(find build/full-release/bin -maxdepth 1 -name zorba -type f | head -n 1)
    test -n "$ZORBA"
    "$ZORBA" --omit-xml-declaration -q "1+1"
    echo
    python3 -I /bench/run.py run --zorba "/src/$ZORBA" --data /data \
      --runs "$RUNS" --timeout "$QUERY_TIMEOUT" --label "$LABEL" \
      --out "/out/$NAME.json" --md "/out/$NAME.md"'

[ -z "${GITHUB_STEP_SUMMARY:-}" ] || cat "out/$NAME.md" >> "$GITHUB_STEP_SUMMARY"
