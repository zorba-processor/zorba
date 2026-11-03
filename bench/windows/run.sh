#!/bin/sh
# Window benchmark. Usage: run.sh ZORBA DIRBENCH WORKDIR OUTDIR
#
# DIRBENCH is bench/directory/dirbench.cpp, compiled with
#   c++ -std=c++23 -O2 -o dirbench bench/directory/dirbench.cpp
# Each cell runs in a fresh process: RUNS times below 10^6 records, once at
# 10^6. It reports the median wall time, the largest peak RSS and the result of
# the query. Environment: SIZES (default "10000 100000 1000000"), RUNS (3),
# LIMIT (seconds per run, 3600), NEVER, NEVER_LINEAR, NEVER_LINEAR_QUERIES.
set -eu
ZORBA=$1 DIRBENCH=$2 WORK=$3 OUT=$4
SIZES=${SIZES:-10000 100000 1000000}
RUNS=${RUNS:-3}
LIMIT=${LIMIT:-3600}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$WORK" "$OUT"
MD=$OUT/results.md
RAW=$OUT/raw.txt
: > "$RAW"
median() { tr ' ' '\n' | grep -v '^$' | sort -n | awk '{ a[NR] = $1 } END {
  if ( NR % 2 ) print a[(NR + 1) / 2]; else printf "%.0f\n", ( a[NR / 2] + a[NR / 2 + 1] ) / 2 }'; }

# cell QUERY SOURCE_KIND SOURCE N [K]
cell() {
  q=$1 kind=$2 src=$3 n=$4 k=${5:-}
  runs=$RUNS
  [ "$n" -ge 1000000 ] && runs=1
  times= rss=0 st=0 r=0
  while [ "$r" -lt "$runs" ]; do
    r=$((r + 1))
    if [ -n "$k" ]; then
      set -- "$DIRBENCH" run "$WORK/out.txt" -- timeout "$LIMIT" "$ZORBA" --omit-xml-declaration \
        -f -q "$HERE/$q.jq" -e "source:=$src" -e "k:=$k"
    else
      set -- "$DIRBENCH" run "$WORK/out.txt" -- timeout "$LIMIT" "$ZORBA" --omit-xml-declaration \
        -f -q "$HERE/$q.jq" -e "source:=$src"
    fi
    line=$("$@")
    set -- $line
    times="$times $1"
    [ "$2" -gt "$rss" ] && rss=$2
    st=$3
    echo "$q $kind $n ${k:--} run$r $line" >> "$RAW"
    [ "$st" -eq 0 ] || break
  done
  t=$(echo "$times" | median)
  m=$(awk -v k="$rss" 'BEGIN { printf "%.0f", k / 1024 }')
  if [ "$st" -eq 0 ]; then res=$(cut -c1-24 "$WORK/out.txt"); else res="exit $st"; fi
  printf '| %s | %s | %s | %s | %s | %s | %s |\n' "$q" "$kind" "$n" "${k:--}" "$t" "$m" "$res" >> "$MD"
}

{
  echo "| query | input | records | k | median ms | peak RSS MiB | result |"
  echo "|---|---|---:|---:|---:|---:|---|"
} > "$MD"

for n in $SIZES; do
  "$ZORBA" --omit-xml-declaration --serialize-text -f -q "$HERE/gen.jq" -e "n:=$n" -e form:=lines > "$WORK/$n.jsonl"
  "$ZORBA" --omit-xml-declaration --serialize-text -f -q "$HERE/gen.jq" -e "n:=$n" -e form:=array > "$WORK/$n.json"
  echo "$n records: $(wc -c < "$WORK/$n.jsonl") bytes of JSON Lines" >> "$RAW"
  for q in baseline moving_points moving_minutes duration sessions; do
    cell "$q" gen "gen:$n" "$n"
    cell "$q" lines "lines:$WORK/$n.jsonl" "$n"
    cell "$q" whole "whole:$WORK/$n.jsonl" "$n"
    cell "$q" array "array:$WORK/$n.json" "$n"
  done
done

# Does the memory of tumbling windows follow the window size or the input?
for n in $SIZES; do
  for k in 10 1000 100000; do
    [ "$k" -le "$n" ] || continue
    cell tumbling_points gen "gen:$n" "$n" "$k"
  done
done

# Windows that stay open, by form. NEVER: sizes with quadratic growth expected in
# some form; NEVER_LINEAR: larger sizes, for the forms that stay linear.
NEVER=${NEVER:-1000 2000 4000 8000 16000}
for n in $NEVER; do
  for q in never_closes never_closes_read never_closes_start open_at_end open_at_end_read first_stays_open tumbling_never; do
    cell "$q" gen "gen:$n" "$n"
  done
done
for n in ${NEVER_LINEAR:-}; do
  for q in ${NEVER_LINEAR_QUERIES:-never_closes never_closes_read open_at_end tumbling_never}; do
    cell "$q" gen "gen:$n" "$n"
  done
done
