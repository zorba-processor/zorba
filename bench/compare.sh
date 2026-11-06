#!/bin/sh
# Compare two zorba builds on one benchmark suite, on this machine, in one go.
# Used by .github/workflows/bench.yml; see bench/README.md.
# Usage: compare.sh BASE_ZORBA HEAD_ZORBA DIRBENCH SUITE_DIR OUTDIR
#
# Every query file (*.xq, *.jq) directly in SUITE_DIR runs as `zorba -f -q FILE ARGS`.
# Per query and build: one warm-up run (run 0, not timed in the statistics) and then
# REPEAT timed runs. The builds alternate within a query, and the build that goes
# first alternates from run to run. Each run is a fresh process under `timeout`.
# A build that fails or times out in the warm-up is not run again on that query.
#
# Writes OUTDIR/raw.tsv (query, build, run, wall ms, peak RSS KiB, status, cksum of
# the output, one row per run) and OUTDIR/errors/ (stderr of the last run of each
# query and build). A failing query is a result, not a script failure: the script
# fails only when it cannot run (bad arguments, no query selected).
#
# Environment:
#   REPEAT   timed runs per query and build (default 5)
#   TIMEOUT  seconds per run (default 300); a run over it has status "timeout"
#   FILTER   space or comma separated query names (file name without extension,
#            shell patterns allowed); empty: all
#   ARGS     extra zorba arguments, split as the shell would, for example
#            "-e n:=100000"; $DATA expands in it
#   DATA     directory the caller prepared for the suite (default OUTDIR/data)
#   BASE_EXEC, HEAD_EXEC
#            command prefix of the runs of that build, for a build that lives in
#            a container: "docker exec zb-base". Everything the runs read or write
#            (DIRBENCH, SUITE_DIR, OUTDIR, the zorba paths, DATA) must then have
#            the same path here and inside it (a bind mount and a symlink do). Default: none (run here).
#   BUDGET   seconds after which no further query is started; the queries left get
#            status "not-run" (default: no limit)
set -eu
if [ "$#" -ne 5 ]; then
  echo "usage: $0 BASE_ZORBA HEAD_ZORBA DIRBENCH SUITE_DIR OUTDIR" >&2
  exit 2
fi
BASE=$1 HEAD=$2 DIRBENCH=$3 SUITE=$4 OUT=$5
REPEAT=${REPEAT:-5}
TIMEOUT=${TIMEOUT:-300}
FILTER=${FILTER:-}
ARGS=${ARGS:-}
BASE_EXEC=${BASE_EXEC:-}
HEAD_EXEC=${HEAD_EXEC:-}
BUDGET=${BUDGET:-}
START=$(date +%s)
mkdir -p "$OUT/errors"
DATA=${DATA:-$OUT/data}
export DATA

case $REPEAT in '' | *[!0-9]*) echo "REPEAT must be a positive integer" >&2; exit 2 ;; esac
case $BUDGET in *[!0-9]*) echo "BUDGET must be a number of seconds" >&2; exit 2 ;; esac
case $TIMEOUT in '' | *[!0-9]*) echo "TIMEOUT must be a positive integer" >&2; exit 2 ;; esac
[ "$REPEAT" -ge 1 ] || { echo "REPEAT must be at least 1" >&2; exit 2; }
[ "$TIMEOUT" -ge 1 ] || { echo "TIMEOUT must be at least 1" >&2; exit 2; }
[ -d "$SUITE" ] || { echo "no such suite directory: $SUITE" >&2; exit 2; }

selected() { # NAME
  [ -n "$FILTER" ] || return 0
  set -f  # the patterns are for case, not for the file system
  for pat in $(echo "$FILTER" | tr ',' ' '); do
    # shellcheck disable=SC2254
    case $1 in $pat) set +f; return 0 ;; esac
  done
  set +f
  return 1
}

QUERIES=
for f in "$SUITE"/*.xq "$SUITE"/*.jq; do
  [ -f "$f" ] || continue
  name=${f##*/}
  name=${name%.*}
  selected "$name" || continue
  QUERIES="$QUERIES $f"
done
if [ -z "$QUERIES" ]; then
  echo "no query of $SUITE matches filter '$FILTER'" >&2
  exit 2
fi

RAW=$OUT/raw.tsv
: > "$RAW"

# run_one BUILD EXEC ZORBA QUERY_FILE NAME RUN; sets $status ("ok", "timeout", "exit-N")
run_one() {
  build=$1 ex=$2 z=$3 q=$4 name=$5 run=$6
  err=$OUT/errors/$name.$build.txt
  # shellcheck disable=SC2086
  eval "set -- $ARGS"
  rc=0
  # shellcheck disable=SC2086
  line=$($ex "$DIRBENCH" run "$OUT/last.out" -- timeout -k 10 "$TIMEOUT" "$z" \
    --omit-xml-declaration -f -q "$q" "$@" 2> "$err") || rc=$?
  if [ "$rc" -ne 0 ]; then
    printf '%s\t%s\t%s\t-\t-\tdriver-failed\t-\n' "$name" "$build" "$run" >> "$RAW"
    status=driver-failed
    return 0
  fi
  # shellcheck disable=SC2086
  set -- $line
  ms=$1 rss=$2 code=$3
  case $code in
    0) status=ok ;;
    124 | 137) status=timeout ;;
    *) status=exit-$code ;;
  esac
  sum=-
  if [ "$status" = ok ]; then
    sum=$(cksum < "$OUT/last.out" | cut -d ' ' -f 1)
  fi
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$name" "$build" "$run" "$ms" "$rss" "$status" "$sum" >> "$RAW"
}

for q in $QUERIES; do
  name=${q##*/}
  name=${name%.*}
  if [ -n "$BUDGET" ] && [ $(( $(date +%s) - START )) -gt "$BUDGET" ]; then
    for b in base head; do
      printf '%s\t%s\t0\t-\t-\tnot-run\t-\n' "$name" "$b" >> "$RAW"
    done
    echo "$name: not run, budget of $BUDGET s reached"
    continue
  fi
  alive_base=1 alive_head=1
  r=0
  while [ "$r" -le "$REPEAT" ]; do
    if [ $((r % 2)) -eq 0 ]; then order="base head"; else order="head base"; fi
    for b in $order; do
      if [ "$b" = base ]; then
        [ "$alive_base" -eq 1 ] || continue
        run_one base "$BASE_EXEC" "$BASE" "$q" "$name" "$r"
        [ "$status" = ok ] || alive_base=0
      else
        [ "$alive_head" -eq 1 ] || continue
        run_one head "$HEAD_EXEC" "$HEAD" "$q" "$name" "$r"
        [ "$status" = ok ] || alive_head=0
      fi
      echo "$name $b run $r: $status"
    done
    r=$((r + 1))
  done
done
rm -f "$OUT/last.out"
