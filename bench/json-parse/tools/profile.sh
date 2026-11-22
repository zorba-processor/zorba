#!/bin/sh
# Where the time and the allocations of one query go.
# Usage: profile.sh ZORBA QUERY LOG OUTDIR
#
# ZORBA should be built with frame pointers (see ../README.md). Needs perf, awk
# and a C compiler; Linux with glibc only. Writes to OUTDIR:
#   parts.txt   samples per part (classify.awk) and the seconds they stand for
#   malloc.txt  malloc-family calls and requested bytes per event, net of startup
# A run over LIMIT seconds (default 900) is stopped.
set -eu
ZORBA=$1 QUERY=$2 LOG=$3 OUT=$4
LIMIT=${LIMIT:-900}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$OUT"
EVENTS=$(wc -l < "$LOG")

cc -O2 -shared -fPIC -o "$OUT/malloc_count.so" "$HERE/malloc_count.c"

start=$(date +%s%N)
timeout "$LIMIT" perf record -q -g --call-graph fp -o "$OUT/perf.data" -- \
  "$ZORBA" -f -q "$QUERY" -e "log:=$LOG" > /dev/null
end=$(date +%s%N)
perf script -i "$OUT/perf.data" 2> /dev/null | awk -f "$HERE/classify.awk" | sort -k2,2nr |
  awk -v s="$(( (end - start) / 1000000 ))" '
    $1 == "total" { printf "%-16s %8d          %7.2f s (perf run)\n", $1, $2, s / 1000; next }
    { printf "%-16s %8d %7s %7.2f s\n", $1, $2, $3, s / 1000 * $3 / 100 }' > "$OUT/parts.txt"

# count_mallocs FILE: "<malloc+calloc+realloc> <bytes>" of the query over FILE
count_mallocs() {
  LD_PRELOAD="$OUT/malloc_count.so" timeout "$LIMIT" "$ZORBA" -f -q "$QUERY" -e "log:=$1" \
    2>&1 > /dev/null | sed -n 's/^MALLOC_COUNT malloc=\([0-9]*\) calloc=\([0-9]*\) realloc=\([0-9]*\) free=[0-9]* bytes=\([0-9]*\).*/\1 \2 \3 \4/p' |
    awk '{ print $1 + $2 + $3, $4 }'
}
: > "$OUT/empty.jsonl"
set -- $(count_mallocs "$LOG") $(count_mallocs "$OUT/empty.jsonl")
awk -v n="$EVENTS" -v c="$1" -v b="$2" -v c0="$3" -v b0="$4" 'BEGIN {
  printf "events %d\nallocations per event %.1f\nbytes per event %.0f\nstartup allocations %d\n",
    n, (c - c0) / n, (b - b0) / n, c0 }' > "$OUT/malloc.txt"
cat "$OUT/parts.txt" "$OUT/malloc.txt"
