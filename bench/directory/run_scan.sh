#!/bin/sh
# What zorba-mcp's background scan costs (see
# .github/workflows/directory-scaling.yml, suite=scan).
# Usage: run_scan.sh ZORBA ZORBA_MCP DIRBENCH WORKDIR OUTDIR "N:KB N:KB ..."
#
# For each tree spec N:KB, generates N files of ~KB KB (gen_tree.xq) and runs
# one session RUNS times with the scan, with --scan-cache-mb 1 and with
# --no-scan (fresh process each, see "dirbench scan"). Writes the medians to
# OUTDIR/results.md and the raw numbers to OUTDIR/raw.txt; fails if a session
# fails. "cache" is what the scan cache accounted for when the server ended
# (its log), and its state what the last answer said.
set -eu
ZORBA=$1 MCP=$2 DIRBENCH=$3 WORK=$4 OUT=$5 TREES=$6
RUNS=${RUNS:-3}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$WORK" "$OUT"
RAW=$OUT/raw.txt
: > "$RAW"

median() { tr ' ' '\n' | grep -v '^$' | sort -n | awk '{ a[NR] = $1 } END {
  if ( NR == 0 ) print "-"; else if ( NR % 2 ) print a[(NR + 1) / 2];
  else printf "%.1f\n", ( a[NR / 2] + a[NR / 2 + 1] ) / 2 }'; }
mib() { awk -v k="$1" 'BEGIN { printf "%.1f", k / 1024 }'; }

MD=$OUT/results.md
{
  echo "| files | mode | handshake ms | first list_documents ms | its bytes | scan complete ms | state | cache bytes | cache bytes per file | RSS after handshake MiB | RSS at end MiB | peak RSS MiB |"
  echo "|---:|---|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|"
} > "$MD"

for spec in $TREES; do
  n=${spec%%:*} kb=${spec#*:}
  name="${n}x${kb}KB"
  tree=$WORK/$name
  if [ ! -d "$tree" ]; then
    "$ZORBA" -f -q "$HERE/gen_tree.xq" -e "out:=$tree" -e "files:=$n" \
      -e "kb:=$kb" > /dev/null
  fi
  test "$(find "$tree" -type f | wc -l)" -eq "$n"

  for mode in scan cache-1 no-scan; do
    case $mode in
      scan) flags= ;;
      cache-1) flags="--scan-cache-mb 1" ;;
      no-scan) flags=--no-scan ;;
    esac
    hs= fl= by= co= r1= r2= pk= ca=
    for _ in $(seq "$RUNS"); do
      set -- $("$DIRBENCH" scan "$OUT/$name-$mode.json" -- \
        "$MCP" --root "$tree" --timeout 0 --max-output 104857600 $flags \
        2> "$OUT/$name-$mode.log")
      echo "$name $mode: $*" >> "$RAW"
      hs="$hs $1" fl="$fl $2" by="$by $3" r1="$r1 $6" r2="$r2 $7" pk="$pk $8"
      st=$5
      if [ "$4" != - ]; then co="$co $4"; fi
      c=$(sed -n 's/.*the scan cache holds \([0-9]*\) bytes.*/\1/p' "$OUT/$name-$mode.log")
      if [ -n "$c" ]; then ca="$ca $c"; fi
      sed -n 's/.*the scan cache holds/  cache:/p' "$OUT/$name-$mode.log" >> "$RAW"
    done
    cache=$(echo $ca | median)
    per=-
    if [ "$cache" != - ]; then
      per=$(awk -v c="$cache" -v n="$n" 'BEGIN { printf "%.0f", c / n }')
    fi
    echo "| $n | $mode | $(echo $hs | median) | $(echo $fl | median) | $(echo $by | median) | $(echo $co | median) | $st | $cache | $per | $(mib "$(echo $r1 | median)") | $(mib "$(echo $r2 | median)") | $(mib "$(echo $pk | median)") |" >> "$MD"
  done
done
cat "$MD"
