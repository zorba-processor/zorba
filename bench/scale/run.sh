#!/bin/sh
# Scale benchmark: semi-structured time series through zorba-mcp's run_query. See
# RESULTS.md and .github/workflows/directory-scaling.yml (suite "scale").
# Usage: run.sh ZORBA_MCP DIRBENCH GEN WORKDIR OUTDIR
#
# Each cell (form, access path, operation) is a query made of prolog.jq, the
# input expression of the access path and ops/<op>.jq. It runs through one
# run_query call in a fresh zorba-mcp session over a root that holds only that
# form. Time is that call; peak RSS is that of the server from wait4.
#
# Pass 1 runs every cell with the default limits (no option but --root), RUNS
# times, or until a run fails. Pass 2 runs each failed cell once with --timeout
# EXT_TIMEOUT --max-memory EXT_MEMORY. On diag/, the query is the sum of seq over
# the expression, under both sets of limits. A query stopped by a limit is a
# result: the script only fails if the generator or the driver does.
#
# Environment: SIZE_MB (the JSON Lines size, in 10^6 bytes), FORMS, INPUTS (form-path
# names, all if empty), OPS, RUNS, DIAG (1 or 0), EXT_TIMEOUT (seconds, 0: no pass 2),
# EXT_MEMORY (bytes), EXT_INPUTS (the inputs of pass 2, all if empty), BUDGET
# (seconds from the start; cells after it are reported as not run).
set -eu
MCP=$1 DIRBENCH=$2 GEN=$3 WORK=$4 OUT=$5
SIZE_MB=${SIZE_MB:-100}
FORMS=${FORMS:-jsonl array dir csv}
INPUTS=${INPUTS:-}
OPS=${OPS:-filter aggregate group_by order_by join tumbling sliding_count sliding_time}
RUNS=${RUNS:-1}
DIAG=${DIAG:-1}
EXT_TIMEOUT=${EXT_TIMEOUT:-1200}
EXT_MEMORY=${EXT_MEMORY:-15032385536}
EXT_INPUTS=${EXT_INPUTS:-}
BUDGET=${BUDGET:-18000}
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$WORK/root
RESP=$OUT/responses
mkdir -p "$WORK" "$RESP"
TSV=$OUT/cells.tsv
SIZES=$OUT/data.tsv
printf 'size_mb\tform\tinput\top\tlimits\trun\tms\trss_kib\tstatus\tresult\n' > "$TSV"
printf 'size_mb\tform\trecords\tbytes\tfiles\tgen_s\n' > "$SIZES"
START=$(date +%s)
elapsed() { echo $(( $(date +%s) - START )); }

file_of() {
  case $1 in
    jsonl) echo data.jsonl ;;
    array) echo data.json ;;
    csv) echo data.csv ;;
  esac
}

paths_of() {
  case $1 in
    jsonl) echo open documents direct lines ;;
    array|csv) echo open documents direct ;;
    dir) echo documents direct lines ;;
  esac
}

# The input expression of FORM and PATH.
input_expr() {
  f=$(file_of "$1")
  case $1-$2 in
    jsonl-open|array-open|csv-open) echo "(dir:open(\$dir:root, \"$f\").content[])" ;;
    *-documents) echo "(dir:documents(\$dir:root).content[])" ;;
    jsonl-direct) echo "(parse-json(file:read-text(\$dir:root || \"/$f\")))" ;;
    jsonl-lines) echo "(file:read-text-lines(\$dir:root || \"/$f\") ! parse-json(\$\$))" ;;
    array-direct) echo "(parse-json(file:read-text(\$dir:root || \"/$f\"))[])" ;;
    csv-direct) echo "(csv:parse(file:read-text(\$dir:root || \"/$f\"), { \"cast-letter-booleans\" : false }))" ;;
    dir-direct) echo "(for \$f in file:list(\$dir:root) order by \$f return parse-json(file:read-text(\$dir:root || \"/\" || \$f)))" ;;
    dir-lines) echo "(for \$f in file:list(\$dir:root) order by \$f return file:read-text-lines(\$dir:root || \"/\" || \$f) ! parse-json(\$\$))" ;;
    *) echo "unknown input $1-$2" >&2; exit 1 ;;
  esac
}

temp_of() {
  if [ "$1" = csv ]; then echo metrics_temp; else echo metrics.temp; fi
}

generate() {
  rm -rf "$ROOT"
  mkdir -p "$ROOT"
  t0=$(date +%s)
  gen_out=$("$GEN" "$1" "${SIZE_MB}000000" "$ROOT")
  set -- "$1" $gen_out
  printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$SIZE_MB" "$1" "$2" "$3" \
    "$(find "$ROOT" -type f | wc -l)" "$(( $(date +%s) - t0 ))" >> "$SIZES"
}

selected() { # LIST NAME
  [ -z "$1" ] && return 0
  case " $1 " in *" $2 "*) return 0 ;; esac
  return 1
}

# One session of one call. Appends a row of cells.tsv and sets $status.
# one_run NAME FORM INPUT OP LIMITS RUN QUERY_FILE ARGS...
one_run() {
  name=$1 form=$2 input=$3 op=$4 limits=$5 run=$6 qf=$7
  shift 7
  outer=$(( ${LIMIT_S} + 300 ))
  resp=$RESP/$name-$limits-$run
  if ! "$DIRBENCH" mcp 1 "$qf" "$resp.json" -- timeout "$outer" "$MCP" --root "$ROOT" "$@" \
       > "$resp.txt" 2> "$resp.log"; then
    status=driver-failed
    printf '%s\t%s\t%s\t%s\t%s\t%s\t-\t-\t%s\t%s\n' "$SIZE_MB" "$form" "$input" "$op" \
      "$limits" "$run" "$status" "$(tail -c 200 "$resp.log" | tr '\t\n' '  ')" >> "$TSV"
    return 0
  fi
  ms=$(sed -n 1p "$resp.txt")
  set -- $(sed -n 2p "$resp.txt")
  rss=$1 failed=$2
  if [ "$failed" -eq 0 ]; then
    status=ok
    result=$(sed -n 's/.*"text":"\([^"]*\)".*/\1/p' "$resp.json" | cut -c1-40)
  elif grep -q 'closed the session' "$resp.log"; then
    status=session-closed
    result=$(grep -v 'closed the session' "$resp.log" | tail -c 200 | tr '\t\n' '  ')
  else
    status=$(sed -n 's/.*\\"code\\":\\"\([^\\]*\)\\".*/\1/p' "$resp.json" | head -n 1)
    [ -n "$status" ] || status=error
    result=$(sed -n 's/.*\\"message\\":\\"\([^"]*\)\\".*/\1/p' "$resp.json" | cut -c1-160)
    [ -n "$result" ] || result=$(tail -c 160 "$resp.log" | tr '\t\n' '  ')
  fi
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$SIZE_MB" "$form" "$input" "$op" \
    "$limits" "$run" "$ms" "$rss" "$status" "$result" >> "$TSV"
}

not_run() {
  printf '%s\t%s\t%s\t%s\t%s\t-\t-\t-\tnot-run\tjob budget of %s s reached\n' \
    "$SIZE_MB" "$1" "$2" "$3" "$4" "$BUDGET" >> "$TSV"
}

# The query of FORM, PATH and OP.
write_query() {
  expr=$(input_expr "$1" "$2")
  temp=$(temp_of "$1")
  {
    cat "$HERE/prolog.jq"
    awk -v input="$expr" -v temp="$temp" '
      BEGIN { gsub(/\\/, "\\\\", input); gsub(/&/, "\\\\&", input) }
      { gsub(/@INPUT@/, input); gsub(/@TEMP@/, temp); print }' "$HERE/ops/$3.jq"
  } > "$4"
}

write_diag_query() {
  {
    cat "$HERE/prolog.jq"
    echo 'sum(for $r in ('
    cat "$1"
    echo ') return $r.seq)'
  } > "$2"
}

failed_default() { # FORM INPUT OP
  awk -F'\t' -v f="$1" -v i="$2" -v o="$3" '
    $2 == f && $3 == i && $4 == o && $5 == "default" && $9 != "ok" { found = 1 }
    END { exit !found }' "$TSV"
}

ext_args() {
  LIMIT_S=$EXT_TIMEOUT
  set_ext="--timeout $EXT_TIMEOUT --max-memory $EXT_MEMORY"
}

# Pass 1: the default limits.
LIMIT_S=30
for form in $FORMS; do
  generated=
  for path in $(paths_of "$form"); do
    selected "$INPUTS" "$form-$path" || continue
    if [ -z "$generated" ]; then
      generate "$form"
      generated=1
    fi
    for op in $OPS; do
      if [ "$(elapsed)" -gt "$BUDGET" ]; then
        not_run "$form" "$form-$path" "$op" default
        continue
      fi
      name=$form-$path-$op
      qf=$OUT/$name.jq
      write_query "$form" "$path" "$op" "$qf"
      r=0
      while [ "$r" -lt "$RUNS" ]; do
        r=$((r + 1))
        one_run "$name" "$form" "$form-$path" "$op" default "$r" "$qf"
        [ "$status" = ok ] || break
      done
    done
  done
  if [ "$form" = jsonl ] && [ "$DIAG" = 1 ] && [ -n "$generated" ]; then
    for d in "$HERE"/diag/*.in; do
      v=$(basename "$d" .in)
      qf=$OUT/diag-$v.jq
      write_diag_query "$d" "$qf"
      LIMIT_S=30
      one_run "diag-$v" jsonl "diag" "$v" default 1 "$qf"
      if [ "$EXT_TIMEOUT" -gt 0 ] && [ "$(elapsed)" -le "$BUDGET" ]; then
        ext_args
        one_run "diag-$v" jsonl "diag" "$v" extended 1 "$qf" $set_ext
      fi
    done
  fi
done

# Pass 2: the cells that failed, with higher limits. Each form gets its share of
# the time left, so that the last forms are not left without any.
if [ "$EXT_TIMEOUT" -gt 0 ]; then
  ext_args
  left=$(echo "$FORMS" | wc -w)
  for form in $FORMS; do
    share=$(( ( BUDGET - $(elapsed) ) / left ))
    left=$((left - 1))
    form_end=$(( $(elapsed) + share ))
    generated=
    for op in $OPS; do
      for path in $(paths_of "$form"); do
        selected "$INPUTS" "$form-$path" || continue
        selected "$EXT_INPUTS" "$form-$path" || continue
        failed_default "$form" "$form-$path" "$op" || continue
        if [ "$(elapsed)" -gt "$form_end" ]; then
          not_run "$form" "$form-$path" "$op" extended
          continue
        fi
        if [ -z "$generated" ]; then
          generate "$form"
          generated=1
        fi
        name=$form-$path-$op
        one_run "$name" "$form" "$form-$path" "$op" extended 1 "$OUT/$name.jq" $set_ext
      done
    done
  done
fi
rm -rf "$ROOT"

# results.md: one row per cell and limits; the median time of the runs that ended
# well, the largest peak RSS, and the status of the last run.
{
  echo "## Scale, $SIZE_MB MB"
  echo
  echo "Data:"
  echo
  echo "| form | records | bytes | files |"
  echo "|---|---:|---:|---:|"
  awk -F'\t' 'NR > 1 && !s[$2]++ { printf "| %s | %s | %s | %s |\n", $2, $3, $4, $5 }' "$SIZES"
  echo
  echo "| input | op | limits | runs | s | peak RSS MiB | status | result or error |"
  echo "|---|---|---|---:|---:|---:|---|---|"
  awk -F'\t' '
    NR == 1 { next }
    {
      k = $3 SUBSEP $4 SUBSEP $5
      if (!(k in n)) order[++c] = k
      n[k]++
      if ($7 != "-") t[k] = t[k] " " $7
      if ($8 != "-" && $8 + 0 > rss[k]) rss[k] = $8 + 0
      st[k] = $9; res[k] = $10
    }
    END {
      for (i = 1; i <= c; i++) {
        k = order[i]; split(k, p, SUBSEP)
        m = split(t[k], a, " ")
        for (x = 1; x <= m; x++) for (y = x + 1; y <= m; y++)
          if (a[y] + 0 < a[x] + 0) { z = a[x]; a[x] = a[y]; a[y] = z }
        med = m ? sprintf("%.2f", a[int((m + 1) / 2)] / 1000) : "-"
        printf "| %s | %s | %s | %d | %s | %s | %s | %s |\n", p[1], p[2], p[3], n[k], med,
          rss[k] ? sprintf("%.0f", rss[k] / 1024) : "-", st[k], res[k]
      }
    }' "$TSV"
  echo
  echo "Cells that did not end with a result: $(awk -F'\t' 'NR > 1 && $9 != "ok"' "$TSV" | wc -l) rows of cells.tsv."
} > "$OUT/results.md"
cat "$OUT/results.md"
