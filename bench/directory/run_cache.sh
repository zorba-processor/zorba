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
#
# With SOAK set, also runs long sessions and appends them to results.md (see
# "dirbench soak"). SOAK is a list of
# FORMATS:SCENARIO:FILES:KB:MAXKB:MAXFILES[:CALLS], with FORMATS separated by
# commas or "all", and CALLS by commas (default SOAK_CALLS, by spaces). It reports
# RSS and the counters on exit for each number of calls. LARGE, a list of
# FORMAT:MIB, checks one file of that size kept by the cache: the last response
# of a session with the cache has the same bytes as without it.
set -eu
ZORBA=$1 MCP=$2 DIRBENCH=$3 WORK=$4 OUT=$5 TREES=$6
RUNS=${RUNS:-3}
SOAK_CALLS=${SOAK_CALLS:-100 1000 10000}
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
             "$MCP" --root "$tree" --timeout 0 --no-scan \
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

# Long sessions: what stays in memory as the calls go on.
SOAK_MD=$OUT/soak.md
if [ -n "${SOAK:-}" ]; then
  {
    echo
    echo "## Long sessions"
    echo
    echo "--no-scan --timeout 0. RSS in MiB. KB:files 0:0 is no cache; hits to bytes retained are the counters on exit."
    echo
    echo "| format | scenario | files x KB | KB:files | calls | RSS at the end | peak RSS | hits | misses | admissions | evictions | bytes retained | failed |"
    echo "|---|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|"
  } > "$SOAK_MD"
  for spec in $SOAK; do
    IFS=: read -r formats scenario files kb max_kb max_files only_calls <<EOF_SPEC
$spec
EOF_SPEC
    [ "$formats" = all ] && formats="xml json csv html"
    for format in $(echo "$formats" | tr , ' '); do
      for calls in $(echo "${only_calls:-$SOAK_CALLS}" | tr , ' '); do
        dir=$WORK/soak-$format-$scenario-$max_kb-$max_files-$calls
        rm -rf "$dir"
        mkdir -p "$dir"
        log=$OUT/soak-$format-$scenario-$max_kb-$max_files-$calls
        if ! (cd "$dir" && "$DIRBENCH" soak "$format" "$scenario" "$files" "$kb" \
              "$calls" "$log.json" -- "$MCP" --root "$dir" --timeout 0 --no-scan \
              --cache-max-file-kb "$max_kb" --cache-max-files "$max_files") \
             > "$log.txt" 2> "$log.log"; then
          echo "soak $format $scenario $max_kb:$max_files $calls: the session failed:" >&2
          tail -n 5 "$log.txt" "$log.log" >&2
          exit 1
        fi
        set -- $(tail -n 1 "$log.txt")
        echo "soak $format $scenario $files $kb $max_kb:$max_files $(tr '\n' ';' < "$log.txt") $(grep 'zorba-mcp: cache:' "$log.log" | cut -d' ' -f3- || true)" >> "$RAW"
        echo "| $format | $scenario | ${files}x$kb | $max_kb:$max_files | $1 | $(awk -v k="$2" 'BEGIN { printf "%.1f", k / 1024 }') | $(awk -v k="$3" 'BEGIN { printf "%.1f", k / 1024 }') | $(counter hits "$log.log") | $(counter misses "$log.log") | $(counter admissions "$log.log") | $(counter evictions "$log.log") | $(counter bytes_retained "$log.log") | $4 |" >> "$SOAK_MD"
        rm -rf "$dir"
      done
    done
  done
  cat "$SOAK_MD" >> "$OUT/results.md"
fi

# One large file: the cache gives the bytes that the file gives.
if [ -n "${LARGE:-}" ]; then
  {
    echo
    echo "## One large file, kept"
    echo
    echo "Five calls of a query over the whole content."
    echo
    echo "| format | MiB | KB:files | peak RSS MiB | hits | misses | admissions | bytes retained | same bytes as no cache |"
    echo "|---|---:|---|---:|---:|---:|---:|---:|---|"
  } >> "$OUT/results.md"
  for spec in $LARGE; do
    format=${spec%:*} mib=${spec#*:}
    dir=$WORK/large-$format
    rm -rf "$dir"
    mkdir -p "$dir"
    if [ "$format" = html ]; then
      awk -v n="$((mib * 45000))" 'BEGIN { print "<html><body>"
        for ( i = 0; i < n; ++i ) print "<p class=\"c1\">" i " text</p>"
        print "</body></html>" }' > "$dir/f.html"
    else
      "$ZORBA" -f -q "$HERE/gen_tree.xq" -e "out:=$dir" -e "files:=1" \
        -e "kb:=$((mib * 1024))" -e "formats:=$format" > /dev/null
    fi
    q=$HERE/large.xq
    max_kb=$((mib * 2048))
    base=$OUT/large-$format-off
    "$DIRBENCH" mcp 5 "$q" "$base.json" -- "$MCP" --root "$dir" --timeout 0 \
      --no-scan --cache-max-file-kb 0 > "$base.txt" 2> "$base.log"
    log=$OUT/large-$format-on
    "$DIRBENCH" mcp 5 "$q" "$log.json" -- "$MCP" --root "$dir" --timeout 0 \
      --no-scan --cache-max-file-kb "$max_kb" --cache-max-files 10 \
      > "$log.txt" 2> "$log.log"
    for f in "$base" "$log"; do
      set -- $(sed -n 2p "$f.txt")
      if [ "$2" -ne 0 ]; then
        echo "large $format: $2 failed calls" >&2
        cat "$f.json" "$f.log" >&2
        exit 1
      fi
    done
    same=no
    cmp -s "$base.json" "$log.json" && same=yes
    set -- $(sed -n 2p "$log.txt")
    echo "| $format | $mib | $max_kb:10 | $(awk -v k="$1" 'BEGIN { printf "%.0f", k / 1024 }') | $(counter hits "$log.log") | $(counter misses "$log.log") | $(counter admissions "$log.log") | $(counter bytes_retained "$log.log") | $same |" >> "$OUT/results.md"
    rm -rf "$dir"
    if [ "$same" != yes ]; then
      echo "large $format: another result with the cache" >&2
      exit 1
    fi
  done
fi
cat "$OUT/results.md"
