# Joins hard-wrapped lines into one line per paragraph or list item, because
# GitHub renders every newline in a release body as a line break.
# Code fences, headings, tables, blank lines and list item starts are kept.
function starts_block(s) {
  return s ~ /^[ \t]*$/ || s ~ /^#/ || s ~ /^[ \t]*\|/ || s ~ /^[ \t]*```/ \
      || s ~ /^[ \t]*([-*+]|[0-9]+\.)[ \t]/ || s ~ /^[ \t]*>/
}
/^[ \t]*```/ { if (have) print buf; have = 0; print; code = !code; next }
code { print; next }
{
  if (have && !starts_block($0) && buf !~ /^#/ && buf !~ /^[ \t]*\|/) {
    line = $0; sub(/^[ \t]+/, "", line); buf = buf " " line; next
  }
  if (have) print buf
  buf = $0; have = 1
}
END { if (have) print buf }
