#!/bin/sh
# Runs every site example query with the zorba CLI.
#
# Usage (inside the Zorba runtime image):
#   run_examples.sh <examples-dir> <output-dir>
#
# For each <section>/<name>.xq or .jq it writes <output-dir>/<section>/<name>.out
# (stdout), .err (stderr) and .rc (exit code). A file <name>.args next to the
# query adds extra command-line arguments. Library modules use the .xqm
# extension and are not run directly. A .jq extension makes zorba parse the
# query as JSONiq.
set -u
src=$(cd "$1" && pwd)
out=$2
mkdir -p "$out"
cd "$src"
find . -type f \( -name '*.xq' -o -name '*.jq' \) | sort | while read -r f; do
  f=${f#./}
  base=${f%.*}
  mkdir -p "$out/$(dirname "$f")"
  args=""
  if [ -f "$base.args" ]; then args=$(cat "$base.args"); fi
  # shellcheck disable=SC2086
  zorba --indent --omit-xml-declaration $args -f -q "$src/$f" \
    > "$out/$base.out" 2> "$out/$base.err"
  echo $? > "$out/$base.rc"
  echo "ran $f (exit $(cat "$out/$base.rc"))"
done
