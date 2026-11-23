#!/bin/sh
# Streaming parse-json benchmark. Usage: run.sh ZORBA DIRBENCH LOG OUTDIR
#
# LOG is a file written by tools/gen.jq. DIRBENCH is bench/directory/dirbench.cpp,
# compiled with  c++ -std=c++23 -O2 -o dirbench bench/directory/dirbench.cpp
# Runs each query of this directory RUNS times (default 3) in a fresh process and
# reports the best wall time, the largest peak RSS, the throughput (MB = 10^6 bytes of
# LOG / best time) and the SHA-256 of the output, which must not change between
# builds. A run over LIMIT seconds (default 600) is stopped and reported.
# With JQ set to a jq binary, also times jq counting the events of LOG.
set -eu
ZORBA=$1 DIRBENCH=$2 LOG=$3 OUT=$4
RUNS=${RUNS:-3}
LIMIT=${LIMIT:-600}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$OUT"
MD=$OUT/results.md
RAW=$OUT/raw.txt
: > "$RAW"
BYTES=$(wc -c < "$LOG")

# measure NAME CMD...: RUNS runs of CMD, output to $OUT/NAME.out
measure() {
  name=$1
  shift
  best= rss=0 st=0 r=0
  while [ "$r" -lt "$RUNS" ]; do
    r=$((r + 1))
    line=$("$DIRBENCH" run "$OUT/$name.out" -- timeout "$LIMIT" "$@")
    echo "$name run$r $line" >> "$RAW"
    ms=${line%% *} rest=${line#* }
    kib=${rest%% *} st=${rest#* }
    [ "$kib" -gt "$rss" ] && rss=$kib
    [ "$st" -eq 0 ] || break
    best=$(awk -v a="${best:-$ms}" -v b="$ms" 'BEGIN { print (b < a ? b : a) }')
  done
  if [ "$st" -eq 0 ]; then
    sum=$(sha256sum "$OUT/$name.out" | cut -c1-16)
    awk -v n="$name" -v ms="$best" -v kib="$rss" -v b="$BYTES" -v s="$sum" 'BEGIN {
      printf "| %s | %.2f | %.0f | %.1f | %s |\n", n, ms / 1000, kib / 1024, b / 1000000 / (ms / 1000), s }' >> "$MD"
  else
    printf '| %s | exit %s | | | |\n' "$name" "$st" >> "$MD"
  fi
}

{
  echo "| query | best s | peak RSS MiB | MB/s | output sha256 |"
  echo "|---|---:|---:|---:|---|"
} > "$MD"

for q in count latency flatten filter_project unmatched; do
  measure "$q" "$ZORBA" -f -q "$HERE/$q.jq" -e "log:=$LOG"
done
if [ -n "${JQ:-}" ]; then
  measure jq_count "$JQ" -n 'reduce inputs as $e (0; . + 1)' "$LOG"
fi
cat "$MD"
