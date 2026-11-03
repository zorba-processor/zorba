#!/bin/sh
# One large JSON array: a filter, an aggregate and a tumbling window over it, and
# where the memory goes. Usage: run_array.sh ZORBA DIRBENCH WORKDIR OUTDIR
#
# DIRBENCH as in run.sh. Each cell is one fresh process. Environment: N (records,
# default 1000000), REPEATS (runs per cell, default 1; the lowest time and the
# largest peak RSS are reported).
set -eu
ZORBA=$1 DIRBENCH=$2 WORK=$3 OUT=$4
N=${N:-1000000}
REPEATS=${REPEATS:-1}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$WORK/tree" "$OUT"
MD=$OUT/results.md

"$ZORBA" --omit-xml-declaration --serialize-text -f -q "$HERE/gen.jq" -e "n:=$N" -e form:=lines > "$WORK/x.jsonl"
"$ZORBA" --omit-xml-declaration --serialize-text -f -q "$HERE/gen.jq" -e "n:=$N" -e form:=array > "$WORK/tree/x.json"
"$ZORBA" --omit-xml-declaration --serialize-text -q "concat('[', string-join(for \$i in 1 to $N return 'null', ','), ']')" > "$WORK/nulls.json"

# measure ZORBA-ARGS...: prints "ms | peak RSS MiB | result".
measure() {
  best= rss=0 st=0 r=0
  while [ "$r" -lt "$REPEATS" ]; do
    r=$((r + 1))
    line=$("$DIRBENCH" run "$WORK/out.txt" -- "$ZORBA" --omit-xml-declaration "$@")
    t=${line%% *}; rest=${line#* }; k=${rest%% *}; st=${rest#* }; st=${st%% *}
    if [ -z "$best" ] || [ "$t" -lt "$best" ]; then best=$t; fi
    [ "$k" -gt "$rss" ] && rss=$k
  done
  mib=$(awk -v k="$rss" 'BEGIN { printf "%.0f", k / 1024 }')
  if [ "$st" -eq 0 ]; then res=$(cut -c1-24 "$WORK/out.txt"); else res="exit $st"; fi
  printf '%s | %s | %s' "$best" "$mib" "$res"
}

{
  echo "records: $N; array file: $(wc -c < "$WORK/tree/x.json") bytes; JSON Lines file: $(wc -c < "$WORK/x.jsonl") bytes"
  echo
  echo "| query | input | ms | peak RSS MiB | result |"
  echo "|---|---|---:|---:|---|"
  for q in filter baseline sessions; do
    for input in "array:$WORK/tree/x.json" "string:$WORK/tree/x.json" "dir:$WORK/tree:x.json" "whole:$WORK/x.jsonl"; do
      echo "| $q | ${input%%:*} | $(measure -f -q "$HERE/$q.jq" -e "source:=$input") |"
    done
  done
  echo
  echo "| where the memory goes | ms | peak RSS MiB | result |"
  echo "|---|---:|---:|---|"
  echo "| process alone: 1 | $(measure -q 1) |"
  echo "| text only: string-length(file:read-text) | $(measure -q "import module namespace file = 'http://expath.org/ns/file'; string-length(file:read-text('$WORK/tree/x.json'))") |"
  echo "| array and members: size(parse-json(file:read-text)) | $(measure -q "jsoniq version \"1.0\"; import module namespace file = 'http://expath.org/ns/file'; size(parse-json(file:read-text('$WORK/tree/x.json')))") |"
  echo "| array of nulls: size(parse-json(file:read-text)) | $(measure -q "jsoniq version \"1.0\"; import module namespace file = 'http://expath.org/ns/file'; size(parse-json(file:read-text('$WORK/nulls.json')))") |"
  echo "| text in a variable: let \$t ... parse-json(\$t)[] twice | $(measure -q "jsoniq version \"1.0\"; import module namespace file = 'http://expath.org/ns/file'; let \$t := file:read-text('$WORK/tree/x.json') return count(parse-json(\$t)[]) + count(parse-json(\$t)[])") |"
  echo "| first member: parse-json(file:read-text)[][1].id | $(measure -q "jsoniq version \"1.0\"; import module namespace file = 'http://expath.org/ns/file'; parse-json(file:read-text('$WORK/tree/x.json'))[][1].id") |"
} > "$MD"
cat "$MD"
