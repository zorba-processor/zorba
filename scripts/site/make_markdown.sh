#!/bin/sh
# Usage: make_markdown.sh <built-site-dir>
#
# Writes <page>.md next to every <page>.html of the built site, converting the
# HTML to GitHub-flavored Markdown with pandoc and scripts/site/markdown.lua.
set -eu

dir=${1:?usage: make_markdown.sh <built-site-dir>}
here=$(cd "$(dirname "$0")" && pwd)

for html in "$dir"/*.html; do
  md=${html%.html}.md
  pandoc --from=html --to=gfm-raw_html --wrap=none \
    --lua-filter="$here/markdown.lua" --output="$md" "$html"
  echo "wrote $(basename "$md")"
done
