#!/bin/sh
# Put the licence files of a binary tarball in place: Zorba's LICENSE.txt and
# NOTICE.txt at the root of <install-dir>, and in
# <install-dir>/share/doc/zorba/third-party/<component>/ those of every
# library that scripts/bundle-binary.sh bundled, as listed in its output
# (<bundle-log>, lines "bundled <name> <- <source>"):
#   - ICU, libxml2 and Xerces-C, built from the zorba-processor forks: the
#     licence and notice files of the fork at the commit pinned in
#     docker/builder/Dockerfile (fetched with git);
#   - any other library (Linux only): /usr/share/doc/<package>/copyright of
#     the Debian package that owns it (dpkg -S).
# Fails when a bundled library has no licence file.
# Usage: bundle-licenses.sh <install-dir> <bundle-log>
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
dir=$(cd "${1:?usage: bundle-licenses.sh <install-dir> <bundle-log>}" && pwd)
log=${2:?usage: bundle-licenses.sh <install-dir> <bundle-log>}
out="$dir/share/doc/zorba/third-party"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

cp "$root/LICENSE.txt" "$root/NOTICE.txt" "$dir/"
mkdir -p "$out"

fork() { # component, repository, commit ARG of docker/builder/Dockerfile, files...
  name=$1 repo=$2 arg=$3
  shift 3
  [ -d "$out/$name" ] && return 0
  commit=$(sed -n "s/^ARG $arg=//p" "$root/docker/builder/Dockerfile")
  git init -q "$work/$name"
  git -C "$work/$name" fetch -q --depth 1 --filter=blob:none "$repo" "$commit"
  mkdir "$out/$name"
  for f in "$@"; do
    git -C "$work/$name" show "FETCH_HEAD:$f" > "$out/$name/$(basename "$f")"
  done
  echo "license $name: $* ($repo $commit)"
}

package() { # library source path
  real=$(readlink -f "$1")
  pkg=$( { dpkg -S "$real" 2>/dev/null || dpkg -S "${real#/usr}" 2>/dev/null; } \
         | head -n 1 | cut -d: -f1)
  [ -n "$pkg" ] && [ -f "/usr/share/doc/$pkg/copyright" ] || return 1
  [ -d "$out/$pkg" ] && return 0
  mkdir "$out/$pkg"
  cp -L "/usr/share/doc/$pkg/copyright" "$out/$pkg/copyright"
  echo "license $pkg: copyright"
}

grep '^bundled ' "$log" > "$work/bundled" || true
while read -r _ lib _ src; do
  case "$lib" in
    libicu*) fork icu https://github.com/zorba-processor/icu.git ICU_COMMIT LICENSE ;;
    libxml2.*) fork libxml2 https://github.com/zorba-processor/libxml2.git LIBXML2_COMMIT Copyright ;;
    libxerces-c*) fork xerces-c https://github.com/zorba-processor/xerces-c.git XERCES_COMMIT LICENSE NOTICE ;;
    *) package "$src" || { echo "FAIL  no licence file for $lib ($src)"; exit 1; } ;;
  esac
done < "$work/bundled"
[ -n "$(ls -A "$out")" ] || { echo "FAIL  $out is empty"; exit 1; }
