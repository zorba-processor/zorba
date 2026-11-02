#!/bin/sh
# Directory scaling benchmark (see .github/workflows/directory-scaling.yml).
# Usage: run.sh ZORBA ZORBA_MCP DIRBENCH WORKDIR OUTDIR "N:KB N:KB ..."
#
# For each tree spec N:KB, generates N files of ~KB KB (gen_tree.xq) and times
#   - zorba --directory: list.xq, scan.xq and aggregate.xq, each RUNS times
#     in a fresh process (median wall time, peak RSS);
#   - zorba-mcp: one session per query (baseline.xq, list.xq, scan.xq,
#     aggregate.xq) calling run_query CALLS times (first call, median of the
#     others, peak RSS of the server).
# Writes OUTDIR/results.md and the raw per-call numbers to OUTDIR/raw.txt;
# fails if any query fails.
set -eu
ZORBA=$1 MCP=$2 DIRBENCH=$3 WORK=$4 OUT=$5 TREES=$6
RUNS=${RUNS:-3}
CALLS=${CALLS:-10}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$WORK" "$OUT"
RAW=$OUT/raw.txt
: > "$RAW"

median() { tr ' ' '\n' | sort -n | awk '{ a[NR] = $1 } END {
  if ( NR == 0 ) print "-"; else if ( NR % 2 ) print a[(NR + 1) / 2];
  else printf "%.1f\n", ( a[NR / 2] + a[NR / 2 + 1] ) / 2 }'; }
mib() { awk -v k="$1" 'BEGIN { printf "%.0f", k / 1024 }'; }

CLI_MD=$OUT/cli.md
MCP_MD=$OUT/mcp.md
SHARE_MD=$OUT/share.md
{
  echo "| tree | files | size | query | median ms | peak RSS MiB | result |"
  echo "|---|---:|---:|---|---:|---:|---|"
} > "$CLI_MD"
{
  echo "| tree | query | call 1 ms | calls 2..$CALLS median ms | peak RSS MiB |"
  echo "|---|---|---:|---:|---:|"
} > "$MCP_MD"
{
  echo "| tree | baseline ms | list ms | scan ms | aggregate ms | listing+parsing share of aggregate |"
  echo "|---|---:|---:|---:|---:|---:|"
} > "$SHARE_MD"

for spec in $TREES; do
  n=${spec%%:*} kb=${spec#*:}
  name="${n}x${kb}KB"
  tree=$WORK/$name
  if [ ! -d "$tree" ]; then
    t0=$(date +%s)
    "$ZORBA" -f -q "$HERE/gen_tree.xq" -e "out:=$tree" -e "files:=$n" \
      -e "kb:=$kb" > /dev/null
    echo "generated $name in $(( $(date +%s) - t0 )) s"
  fi
  count=$(find "$tree" -type f | wc -l)
  test "$count" -eq "$n"
  size=$(du -sh "$tree" | cut -f1)

  for q in list scan aggregate; do
    times=
    rss=0
    for _ in $(seq "$RUNS"); do
      set -- $("$DIRBENCH" run "$OUT/cli-$name-$q.out" -- \
        "$ZORBA" --directory "$tree" -f -q "$HERE/$q.xq")
      if [ "$3" -ne 0 ]; then
        echo "zorba --directory $tree $q.xq failed (exit $3):" >&2
        cat "$OUT/cli-$name-$q.out" >&2
        exit 1
      fi
      times="$times $1"
      if [ "$2" -gt "$rss" ]; then rss=$2; fi
    done
    echo "cli $name $q:$times rss=$rss" >> "$RAW"
    result=$(tr -d '\n' < "$OUT/cli-$name-$q.out" | cut -c1-60)
    echo "| $name | $n | $size | $q | $(echo $times | median) | $(mib $rss) | \`$result\` |" >> "$CLI_MD"
  done

  for q in baseline list scan aggregate; do
    if ! "$DIRBENCH" mcp "$CALLS" "$HERE/$q.xq" "$OUT/mcp-$name-$q.json" -- \
         "$MCP" --root "$tree" --timeout 0 > "$OUT/mcp-$name-$q.txt" \
         2> "$OUT/mcp-$name-$q.log"; then
      echo "zorba-mcp $tree $q.xq: the session failed:" >&2
      cat "$OUT/mcp-$name-$q.log" >&2
      exit 1
    fi
    times=$(sed -n 1p "$OUT/mcp-$name-$q.txt")
    set -- $(sed -n 2p "$OUT/mcp-$name-$q.txt")
    rss=$1 failed=$2
    echo "mcp $name $q: $times rss=$rss failed=$failed" >> "$RAW"
    if [ "$failed" -ne 0 ]; then
      echo "zorba-mcp $tree $q.xq: $failed failed calls:" >&2
      cat "$OUT/mcp-$name-$q.json" "$OUT/mcp-$name-$q.log" >&2
      exit 1
    fi
    first=${times%% *}
    rest=$(echo "$times" | cut -d' ' -f2- | median)
    eval "rest_$q=\$rest"
    echo "| $name | $q | $first | $rest | $(mib "$rss") |" >> "$MCP_MD"
  done
  share=$(awk -v b="$rest_baseline" -v s="$rest_scan" -v a="$rest_aggregate" \
    'BEGIN { printf "%.0f%%", 100 * ( s - b ) / a }')
  echo "| $name | $rest_baseline | $rest_list | $rest_scan | $rest_aggregate | $share |" >> "$SHARE_MD"
done

{
  echo "## Directory scaling"
  echo
  echo "zorba --directory: one process per run, median of $RUNS runs."
  echo
  cat "$CLI_MD"
  echo
  echo "zorba-mcp: one session per query, $CALLS run_query calls each."
  echo
  cat "$MCP_MD"
  echo
  echo "Subsequent zorba-mcp calls (median of calls 2..$CALLS). Listing+parsing share = (scan - baseline) / aggregate."
  echo
  cat "$SHARE_MD"
} > "$OUT/results.md"
cat "$OUT/results.md"
