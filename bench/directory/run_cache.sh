#!/bin/sh
# What the zorba-mcp document cache costs and gains (see
# .github/workflows/directory-scaling.yml, suite "cache").
# Usage: run_cache.sh ZORBA ZORBA_MCP DIRBENCH WORKDIR OUTDIR "SPEC ..."
#
# Each SPEC is a tree: N:KB (N files of ~KB KB, XML, JSON and CSV in turn),
# N:KB:FORMATS (gen_tree.xq) or N:KB:@SHAPE (gen_worst.xq, scan.xq only).
#
# For each tree and query, runs one session per setting in CACHE ("KB:FILES",
# for --cache-max-file-kb and --cache-max-files; the first setting is what the
# others are compared with), RUNS times in rotating order, so that all settings
# share the same runner and its drift. Each session makes CALLS run_query
# calls. Reports the medians over the runs of call 1 and of the median of the
# later calls, the highest peak RSS, and the counters of the last run.
# Writes OUTDIR/results.md and OUTDIR/raw.txt; fails if any query fails.
set -eu
ZORBA=$1 MCP=$2 DIRBENCH=$3 WORK=$4 OUT=$5 TREES=$6
RUNS=${RUNS:-3}
CALLS=${CALLS:-6}
CACHE=${CACHE:-0:0 16:10 64:10 256:10 64:1000}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$WORK" "$OUT"
RAW=$OUT/raw.txt
: > "$RAW"

median() { tr ' ' '\n' | grep . | sort -n | awk '{ a[NR] = $1 } END {
  if ( NR == 0 ) print "-"; else if ( NR % 2 ) print a[(NR + 1) / 2];
  else printf "%.1f\n", ( a[NR / 2] + a[NR / 2 + 1] ) / 2 }'; }
counter() { sed -n "s/.*zorba-mcp: cache: .*$1=\([0-9]*\).*/\1/p" "$2" | tail -n 1; }
percent() { awk -v a="$1" -v b="$2" 'BEGIN { printf "%+.1f%%", 100 * ( a - b ) / b }'; }

MD=$OUT/cache.md
{
  echo "| tree | query | KB:files | call 1 ms | vs first | later calls ms | vs first | peak RSS MiB | hits | misses | admissions | evictions | bytes retained | result |"
  echo "|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|"
} > "$MD"

for spec in $TREES; do
  n=${spec%%:*} opts=${spec#*:}
  kb=${opts%%:*} formats=
  name="${n}x${kb}KB"
  if [ "$opts" != "$kb" ]; then
    formats=${opts#*:}
    name="$name-$(echo "$formats" | tr -d @ | tr , -)"
  fi
  tree=$WORK/$name
  queries="scan aggregate"
  case $formats in @*) queries=scan ;; esac
  if [ ! -d "$tree" ]; then
    case $formats in
      @*) "$ZORBA" -f -q "$HERE/gen_worst.xq" -e "out:=$tree" -e "files:=$n" \
            -e "kb:=$kb" -e "shape:=${formats#@}" > /dev/null ;;
      *)  "$ZORBA" -f -q "$HERE/gen_tree.xq" -e "out:=$tree" -e "files:=$n" \
            -e "kb:=$kb" -e "formats:=${formats:-xml,json,csv}" > /dev/null ;;
    esac
  fi
  test "$(find "$tree" -type f | wc -l)" -eq "$n"

  for q in $queries; do
    order=$CACHE
    for run in $(seq "$RUNS"); do
      for setting in $order; do
        log=$OUT/$name-$q-$(echo "$setting" | tr : _)-$run
        if ! "$DIRBENCH" mcp "$CALLS" "$HERE/$q.xq" "$log.json" -- \
             "$MCP" --root "$tree" --timeout 0 \
             --cache-max-file-kb "${setting%:*}" \
             --cache-max-files "${setting#*:}" > "$log.txt" 2> "$log.log"; then
          echo "zorba-mcp $tree $q.xq $setting: the session failed:" >&2
          cat "$log.log" >&2
          exit 1
        fi
        times=$(sed -n 1p "$log.txt")
        set -- $(sed -n 2p "$log.txt")
        if [ "$2" -ne 0 ]; then
          echo "zorba-mcp $tree $q.xq $setting: $2 failed calls:" >&2
          cat "$log.json" "$log.log" >&2
          exit 1
        fi
        echo "$name $q $setting $run first=${times%% *} later=$(echo "$times" | cut -d' ' -f2- | median) rss=$1 $(grep 'zorba-mcp: cache:' "$log.log" | cut -d' ' -f3- || true) calls: $times" >> "$RAW"
      done
      # The next run starts with the setting after this run's first.
      order="$(echo "$order" | cut -d' ' -f2-) $(echo "$order" | cut -d' ' -f1)"
    done

    base_first= base_later=
    for setting in $CACHE; do
      lines=$(grep "^$name $q $setting " "$RAW")
      first=$(echo "$lines" | sed 's/.* first=\([^ ]*\).*/\1/' | tr '\n' ' ' | median)
      later=$(echo "$lines" | sed 's/.* later=\([^ ]*\).*/\1/' | tr '\n' ' ' | median)
      rss=$(echo "$lines" | sed 's/.* rss=\([^ ]*\).*/\1/' | sort -n | tail -n 1)
      if [ -z "$base_first" ]; then base_first=$first base_later=$later; fi
      log=$OUT/$name-$q-$(echo "$setting" | tr : _)-$RUNS
      result=$(grep -o '"text":"[^"]*"' "$log.json" | tail -n 1 | cut -c1-40)
      echo "| $name | $q | $setting | $first | $(percent "$first" "$base_first") | $later | $(percent "$later" "$base_later") | $(awk -v k="$rss" 'BEGIN { printf "%.0f", k / 1024 }') | $(counter hits "$log.log") | $(counter misses "$log.log") | $(counter admissions "$log.log") | $(counter evictions "$log.log") | $(counter bytes_retained "$log.log") | \`$result\` |" >> "$MD"
    done
  done
  # Only the logs of the last run and raw.txt are worth keeping.
  find "$OUT" -name "$name-*" ! -name "*-$RUNS.log" ! -name "*-$RUNS.json" -delete
done

{
  echo "## zorba-mcp document cache"
  echo
  echo "One session per setting, $RUNS runs in rotating order on this runner, $CALLS run_query calls per session."
  echo
  cat "$MD"
} > "$OUT/results.md"
cat "$OUT/results.md"
