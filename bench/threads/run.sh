#!/bin/sh
# Multithreaded throughput benchmark. Usage: run.sh MTBENCH ZORBA XMARK LOG OUTDIR
#
# MTBENCH is mtbench.cpp built against the libzorba under test, ZORBA the zorba
# binary of the same build (it writes the report). XMARK is an XMark document
# (bench/gen_data.sh), LOG a log written by ../json-parse/tools/gen.jq.
# Runs each query with mtbench at the thread counts THREADS (default
# 1,2,4,8,16; a build with ZORBA_FOR_ONE_THREAD_ONLY takes only 1) for DURATION
# seconds each (default 5). A query whose mtbench takes longer than LIMIT
# seconds (default 900) is stopped and reported. Writes OUTDIR/runs.jsonl and
# OUTDIR/results.md; with BASE set to the runs.jsonl of another build, the
# report also gives the time per query relative to that build.
set -eu
MTBENCH=$1 ZORBA=$2 XMARK=$3 LOG=$4 OUT=$5
THREADS=${THREADS:-1,2,4,8,16}
DURATION=${DURATION:-5}
LIMIT=${LIMIT:-900}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$OUT"
RUNS=$OUT/runs.jsonl
: > "$RUNS"
status=0

# bench NAME QUERY ARGS...
bench() {
  name=$1 query=$2
  shift 2
  if ! timeout "$LIMIT" "$MTBENCH" --name "$name" --threads "$THREADS" \
      --seconds "$DURATION" "$@" "$query" >> "$RUNS"; then
    echo "$name: failed or stopped after $LIMIT s" >&2
    status=1
  fi
}

for q in q01 q08 q14 q19; do
  bench "xmark/$q" "$HERE/../xmark/$q.xq" -x "auction=$XMARK"
  bench "xmark/$q/private" "$HERE/../xmark/$q.xq" -x "auction=$XMARK" --private
done
for q in count latency filter_project; do
  bench "json/$q" "$HERE/../json-parse/$q.jq" -e "log:=$LOG"
done
bench validate "$HERE/validate.xq"
bench api/atomics "$HERE/atomics.xq" --items

"$ZORBA" --serialize-text -f -q "$HERE/report.xq" -e "runs:=$RUNS" \
  -e "cpus:=$(nproc)" ${BASE:+-e "base:=$BASE"} > "$OUT/results.md"
cat "$OUT/results.md"
exit "$status"
