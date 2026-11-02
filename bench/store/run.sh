#!/bin/sh
# Store memory benchmark (see .github/workflows/store-memory.yml).
# Usage: run.sh ZORBA STOREBENCH SIZES WORKDIR OUTDIR "MB MB ..."
#
# For XML, JSON and CSV and each size in MiB, generates one file (gen.xq)
# and parses it RUNS times with storebench, one process per run; then runs
# the microbenchmarks (MICRO_N items per document) RUNS times. SIZES is the
# "class sizeof" list from sizes.cpp. Writes OUTDIR/runs.jsonl,
# OUTDIR/micro.jsonl and OUTDIR/results.md (report.xq); fails if any step
# fails.
set -eu
ZORBA=$1 BENCH=$2 SIZES=$3 WORK=$4 OUT=$5 MBS=$6
RUNS=${RUNS:-3}
MICRO_N=${MICRO_N:-200000}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$WORK" "$OUT"
: > "$OUT/runs.jsonl"
: > "$OUT/micro.jsonl"

for format in xml json csv; do
  for mb in $MBS; do
    f=$WORK/$format-$mb.$format
    if [ ! -f "$f" ]; then
      t0=$(date +%s)
      "$ZORBA" -f -q "$HERE/gen.xq" -e "out:=$f" -e "format:=$format" \
        -e "mb:=$mb" > /dev/null
      echo "generated $f ($(wc -c < "$f") bytes) in $(( $(date +%s) - t0 )) s"
    fi
    for r in $(seq "$RUNS"); do
      "$BENCH" file "$format" "$f" >> "$OUT/runs.jsonl"
      echo "$format $mb MiB run $r: $(tail -n 1 "$OUT/runs.jsonl" | cut -c1-200)"
    done
  done
done

for r in $(seq "$RUNS"); do
  "$BENCH" micro "$MICRO_N" >> "$OUT/micro.jsonl"
  echo "micro run $r done"
done

"$ZORBA" --serialize-text -f -q "$HERE/report.xq" -e "runs:=$OUT/runs.jsonl" \
  -e "micro:=$OUT/micro.jsonl" -e "sizes:=$SIZES" > "$OUT/results.md"
cat "$OUT/results.md"
