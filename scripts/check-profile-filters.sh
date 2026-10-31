#!/bin/sh
# Fails if a tracked file that contains a build-profile guard macro is not
# matched by the patterns of the CI profile that toggles that macro
# (.github/ci-filters.yml, used by .github/workflows/ci.yml). Without this
# check a guard added in a new place would silently stop triggering the
# profile that exercises it.
set -eu
cd "$(dirname "$0")/.."
F=.github/ci-filters.yml

# profile: macros it toggles (embedded-noicu also inherits embedded's).
MAP='embedded:ZORBA_NO_FULL_TEXT ZORBA_NO_XMLSCHEMA ZORBA_WITH_DEBUGGER ZORBA_WITH_THESAURUS ZORBA_SUPPRESS_CURL ZORBA_HAVE_CURL
embedded-noicu:ZORBA_NO_FULL_TEXT ZORBA_NO_XMLSCHEMA ZORBA_WITH_DEBUGGER ZORBA_WITH_THESAURUS ZORBA_SUPPRESS_CURL ZORBA_HAVE_CURL ZORBA_NO_ICU ZORBA_NO_UNICODE
bigint:ZORBA_WITH_BIG_INTEGER
full-nopch:ZORBA_WITH_DEBUGGER ZORBA_XQUERYX ZORBA_TEST_XQUERYX ZORBA_USE_PCH'

# Expand the filter file into "profile<TAB>regex" lines (glob -> ERE,
# YAML anchors/aliases of the form "key: &name" / "- *name" resolved).
patterns() {
  awk -v q="'" '
    function re(g,   r, i, c, n) {
      r = "^"; n = length(g)
      for (i = 1; i <= n; i++) {
        c = substr(g, i, 1)
        if (c == "*" && substr(g, i, 3) == "**/") { r = r "(.*/)?"; i += 2 }
        else if (c == "*" && substr(g, i, 2) == "**") { r = r ".*"; i += 1 }
        else if (c == "*") r = r "[^/]*"
        else if (c == "?") r = r "[^/]"
        else if (c ~ /[.+(){}|^$\[\]\\]/) r = r "\\" c
        else r = r c
      }
      return r "$"
    }
    function emit(k, list,   a, n, i) {
      n = split(list, a, "\n")
      for (i = 1; i <= n; i++) if (a[i] != "") print k "\t" a[i]
    }
    /^[A-Za-z][A-Za-z0-9-]*:/ {
      key = $1; sub(/:$/, "", key); anchor = ""
      if ($2 ~ /^&/) { anchor = substr($2, 2); name[anchor] = key }
      list[key] = ""; next
    }
    /^[ ]+- \*/ { a = $2; sub(/^\*/, "", a); list[key] = list[key] list[name[a]]; next }
    /^[ ]+- / { g = $0; sub(/^[ ]+- /, "", g); if (substr(g, 1, 1) == q) g = substr(g, 2, length(g) - 2); list[key] = list[key] re(g) "\n" }
    END { for (k in list) emit(k, list[k]) }
  ' "$F"
}
P=$(patterns)

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
: > "$tmp/missing"
echo "$MAP" | while IFS=: read -r profile macros; do
  printf '%s\n' "$P" | awk -F'\t' -v p="$profile" '$1 == p { print $2 }' > "$tmp/pats"
  [ -s "$tmp/pats" ] || { echo "no patterns for profile $profile" >> "$tmp/missing"; continue; }
  for m in $macros; do
    git grep -lw "$m" -- . ':!doc' ':!site' ':!.github' ':!.superpowers' \
        ':!CHANGELOG.md' ':!README.md' ':!.clusterfuzzlite' ':!scripts/check-os-access.allow' ':!changelog.d' ':!scripts/check-profile-filters.sh' \
        ':!**/[Rr][Ee][Aa][Dd][Mm][Ee]*' ||:
  done | sort -u | while read -r f; do
    printf '%s\n' "$f" | grep -Eqf "$tmp/pats" ||
      echo "profile $profile: $f (guard macro) is not matched by $F" >> "$tmp/missing"
  done
done
if [ -s "$tmp/missing" ]; then cat "$tmp/missing"; exit 1; fi
echo "profile filters cover all guard macros"
