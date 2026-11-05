#!/bin/sh
# Helper of .github/workflows/quick-check.yml.
#
#   scripts/quick-check.sh run       inside the builder container: configure,
#                                    build, then run the selected tests.
#   scripts/quick-check.sh summary   on the runner: print the Markdown job
#                                    summary from the files `run` wrote.
#
# `run` reads QC_PRESET, QC_TESTS (ctest -R regex, empty = build only),
# QC_CTEST_ARGS (extra ctest arguments, word-split) and QC_TIMEOUT (seconds
# per test). Its files go to build/quick-check/: ctest.log (full output),
# times.txt and ccache.txt. `summary` truncates every line it prints, so that
# a test printing a huge single line cannot flood the job summary.
set -u

out=build/quick-check
max_line=300     # characters kept per summary line
max_lines=200    # lines of failed-test output kept in the summary

run() {
  : "${QC_PRESET:?}" "${QC_TIMEOUT:=300}"
  mkdir -p "$out"
  : > "$out/times.txt"
  t0=$(date +%s)
  cmake --preset "$QC_PRESET" || exit 1
  t1=$(date +%s)
  echo "configure: $((t1 - t0)) s" >> "$out/times.txt"
  cmake --build --preset "$QC_PRESET" || exit 1
  t2=$(date +%s)
  echo "build: $((t2 - t1)) s" >> "$out/times.txt"
  ccache -s > "$out/ccache.txt" 2>&1
  cat "$out/ccache.txt"
  if [ -z "${QC_TESTS:-}" ]; then
    echo "tests: none selected (build only)" >> "$out/times.txt"
    chown_out
    exit 0
  fi
  mkdir -p "build/$QC_PRESET/sanitizer-logs"
  rc=0
  # --no-tests=error: a regex that matches nothing must fail, not pass.
  # shellcheck disable=SC2086
  ctest --preset "$QC_PRESET" -R "$QC_TESTS" -j"$(nproc)" \
    --timeout "$QC_TIMEOUT" --no-tests=error --output-on-failure \
    --output-junit junit.xml ${QC_CTEST_ARGS:-} \
    > "$out/ctest.log" 2>&1 || rc=$?
  t3=$(date +%s)
  echo "ctest: $((t3 - t2)) s" >> "$out/times.txt"
  # Console: the failures and the summary, one line capped at max_line.
  cut -c1-"$max_line" "$out/ctest.log" | tail -n 60
  chown_out
  exit "$rc"
}

# The sanitizer runtime and root-in-container leave root-owned files.
chown_out() {
  if [ -n "${HOST_UID:-}" ]; then
    chown -R "$HOST_UID:${HOST_GID:-$HOST_UID}" "$out" "build/$QC_PRESET" .ccache 2> /dev/null || true
  fi
}

summary() {
  echo "## Quick check"
  echo
  echo "Preset \`${QC_PRESET:-?}\`, tests \`${QC_TESTS:-}\`. This is not the CI gate: the final push still needs the full CI."
  echo
  if [ -s "$out/times.txt" ]; then
    echo '```'
    cat "$out/times.txt"
    grep -E 'Hits:|Misses:|Cacheable calls' "$out/ccache.txt" 2> /dev/null | cut -c1-"$max_line"
    echo '```'
    echo
  else
    echo "No build output: the run failed before or during configure or build."
    return 0
  fi
  [ -s "$out/ctest.log" ] || return 0
  echo '```'
  grep -E 'tests passed|tests failed out of|No tests were found' "$out/ctest.log" | cut -c1-"$max_line"
  sed -n '/^The following tests FAILED/,$p' "$out/ctest.log" | cut -c1-"$max_line" | head -n 60
  echo '```'
  # Output of the failed tests: ctest prints it right after the result line.
  awk -v w="$max_line" -v m="$max_lines" '
    /^ *[0-9]+\/[0-9]+ Test +#[0-9]+: / { show = ($0 ~ /\*\*\*|Failed|Timeout|Exception/); }
    /^[0-9]+% tests passed/ { show = 0 }
    show && n < m { print substr($0, 1, w); n++ }
    show && n == m { print "... (truncated, see the quick-check-log artifact)"; n++ }
  ' "$out/ctest.log" | { read -r first || exit 0; echo; echo "### Failed tests"; echo '```'; echo "$first"; cat; echo '```'; }
}

case "${1:-}" in
  run) run ;;
  summary) summary ;;
  *) echo "usage: $0 run|summary" >&2; exit 2 ;;
esac
