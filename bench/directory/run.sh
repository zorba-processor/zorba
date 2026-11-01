#!/bin/sh
# Directory scaling benchmark (see .github/workflows/directory-scaling.yml).
# Usage: run.sh ZORBA ZORBA_MCP DIRBENCH WORKDIR OUTDIR "N:KB[:FORMATS] ..."
#
# For each tree spec N:KB, generates N files of ~KB KB (gen_tree.xq; XML,
# JSON and CSV in turn, or the comma-separated FORMATS) and times
#   - zorba --directory: list.xq, scan.xq, aggregate.xq and hold.xq, each
#     RUNS times in a fresh process (median wall time, peak RSS);
#   - zorba-mcp, for each --cache-mb value in CACHE_MBS (default "0 256"):
#     one session per query (baseline.xq, list.xq, scan.xq, aggregate.xq)
#     calling run_query CALLS times (first call, median of the others, peak
#     RSS of the server).
# hold.xq keeps every parsed document: (its RSS - list RSS) / file bytes is
# the in-memory size of a parsed document per byte of file.
# Writes OUTDIR/results.md and the raw per-call numbers to OUTDIR/raw.txt;
# fails if any query fails.
set -eu
ZORBA=$1 MCP=$2 DIRBENCH=$3 WORK=$4 OUT=$5 TREES=$6
RUNS=${RUNS:-3}
CALLS=${CALLS:-10}
CACHE_MBS=${CACHE_MBS:-0 256}
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
HOLD_MD=$OUT/hold.md
{
  echo "| tree | files | size | query | median ms | peak RSS MiB | result |"
  echo "|---|---:|---:|---|---:|---:|---|"
} > "$CLI_MD"
{
  echo "| tree | cache MiB | query | call 1 ms | calls 2..$CALLS median ms | peak RSS MiB |"
  echo "|---|---:|---|---:|---:|---:|"
} > "$MCP_MD"
{
  echo "| tree | cache MiB | baseline ms | list ms | scan ms | aggregate ms | listing+parsing share of aggregate |"
  echo "|---|---:|---:|---:|---:|---:|---:|"
} > "$SHARE_MD"
{
  echo "| tree | file bytes | list RSS MiB | scan RSS MiB | hold RSS MiB | (hold - list) / file bytes |"
  echo "|---|---:|---:|---:|---:|---:|"
} > "$HOLD_MD"

for spec in $TREES; do
  n=${spec%%:*} opts=${spec#*:}
  kb=${opts%%:*} formats=xml,json,csv
  name="${n}x${kb}KB"
  if [ "$opts" != "$kb" ]; then
    formats=${opts#*:}
    name="$name-$(echo "$formats" | tr , -)"
  fi
  tree=$WORK/$name
  if [ ! -d "$tree" ]; then
    t0=$(date +%s)
    "$ZORBA" -f -q "$HERE/gen_tree.xq" -e "out:=$tree" -e "files:=$n" \
      -e "kb:=$kb" -e "formats:=$formats" > /dev/null
    echo "generated $name in $(( $(date +%s) - t0 )) s"
  fi
  count=$(find "$tree" -type f | wc -l)
  test "$count" -eq "$n"
  size=$(du -sh "$tree" | cut -f1)
  bytes=$(find "$tree" -type f -exec cat {} + | wc -c)

  for q in list scan aggregate hold; do
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
    eval "rss_$q=\$rss"
    result=$(tr -d '\n' < "$OUT/cli-$name-$q.out" | cut -c1-60)
    echo "| $name | $n | $size | $q | $(echo $times | median) | $(mib $rss) | \`$result\` |" >> "$CLI_MD"
  done

  awk -v b="$bytes" -v l="$rss_list" -v s="$rss_scan" -v h="$rss_hold" \
    -v name="$name" 'BEGIN { printf "| %s | %d | %.0f | %.0f | %.0f | %.1f |\n",
      name, b, l / 1024, s / 1024, h / 1024, ( h - l ) * 1024 / b }' >> "$HOLD_MD"

  for mb in $CACHE_MBS; do
  for q in baseline list scan aggregate; do
    log=$OUT/mcp-$name-c$mb-$q
    if ! "$DIRBENCH" mcp "$CALLS" "$HERE/$q.xq" "$log.json" -- \
         "$MCP" --root "$tree" --timeout 0 --cache-mb "$mb" > "$log.txt" \
         2> "$log.log"; then
      echo "zorba-mcp $tree $q.xq: the session failed:" >&2
      cat "$log.log" >&2
      exit 1
    fi
    times=$(sed -n 1p "$log.txt")
    set -- $(sed -n 2p "$log.txt")
    rss=$1 failed=$2
    echo "mcp $name cache=$mb $q: $times rss=$rss failed=$failed" >> "$RAW"
    if [ "$failed" -ne 0 ]; then
      echo "zorba-mcp $tree $q.xq: $failed failed calls:" >&2
      cat "$log.json" "$log.log" >&2
      exit 1
    fi
    first=${times%% *}
    rest=$(echo "$times" | cut -d' ' -f2- | median)
    eval "rest_$q=\$rest"
    echo "| $name | $mb | $q | $first | $rest | $(mib "$rss") |" >> "$MCP_MD"
  done
  share=$(awk -v b="$rest_baseline" -v s="$rest_scan" -v a="$rest_aggregate" \
    'BEGIN { printf "%.0f%%", 100 * ( s - b ) / a }')
  echo "| $name | $mb | $rest_baseline | $rest_list | $rest_scan | $rest_aggregate | $share |" >> "$SHARE_MD"
  done
done

{
  echo "## Directory scaling"
  echo
  echo "zorba --directory: one process per run, median of $RUNS runs."
  echo
  cat "$CLI_MD"
  echo
  echo "Memory of the parsed documents (peak RSS of zorba --directory)."
  echo
  cat "$HOLD_MD"
  echo
  echo "zorba-mcp: one session per query and --cache-mb value, $CALLS run_query calls each."
  echo
  cat "$MCP_MD"
  echo
  echo "Subsequent zorba-mcp calls (median of calls 2..$CALLS). Listing+parsing share = (scan - baseline) / aggregate."
  echo
  cat "$SHARE_MD"
} > "$OUT/results.md"
cat "$OUT/results.md"
