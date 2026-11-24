#!/bin/sh
# Make an installed Zorba tree (cmake --install) self-contained and
# relocatable: copy into <dir>/lib every shared library that its executables
# and libraries load from outside the system baseline, and make each file
# find its libraries in <dir>/lib relative to its own location.
#   Linux: the baseline is glibc, libstdc++ and libgcc_s; every ELF file gets
#          the RUNPATH $ORIGIN/<path to lib> (patchelf).
#   macOS: the baseline is /usr/lib and /System; every library is referenced
#          as @rpath/<name>, every Mach-O file gets the single LC_RPATH
#          @loader_path/<path to lib> (install_name_tool) and is then signed
#          ad hoc (codesign -s -).
# Prints "bundled <name> <- <source>" per copied library and fails when a
# file still loads a library from outside the tree and the baseline.
# Usage: bundle-binary.sh <install-dir>
set -eu
dir=$(cd "${1:?usage: bundle-binary.sh <install-dir>}" && pwd)
lib="$dir/lib"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# Path from the directory of file $1 (inside the tree) to $lib, for $ORIGIN
# and @loader_path.
rel_to_lib() {
  sub=$(dirname "${1#"$dir"/}")
  case "$sub" in
    lib) echo . ;;
    lib/*) echo "${sub#lib/}" | sed 's|[^/]*|..|g' ;;
    *) echo "$sub" | sed 's|[^/]*|..|g; s|$|/lib|' ;;
  esac
}

linux_bundle() {
  baseline='^(linux-vdso|linux-gate|libc|libm|libdl|libpthread|librt|libresolv|libutil|libanl|libgcc_s|libstdc\+\+)\.so'
  find "$dir/bin" "$dir/lib" -type f -exec sh -c 'readelf -h "$1" >/dev/null 2>&1' _ {} \; -print > "$work/elf"
  # Search paths first, so that ldd resolves the tree's own libraries.
  while read -r f; do
    patchelf --set-rpath "\$ORIGIN/$(rel_to_lib "$f")" "$f"
  done < "$work/elf"
  # ldd lists the whole dependency closure, so one pass finds every library.
  # The dynamic loader is the one entry named by its path; it is skipped.
  while read -r f; do
    ldd "$f" | awk '$2 == "=>" && $1 !~ /\// { print $1, $3 }'
  done < "$work/elf" | sort -u > "$work/deps"
  status=0
  while read -r name path; do
    if echo "$name" | grep -Eq "$baseline"; then continue; fi
    case "$path" in
      not) echo "FAIL  $name: not found"; status=1; continue ;;
      "$dir"/*) continue ;;
    esac
    if [ ! -e "$lib/$name" ]; then
      cp -L "$path" "$lib/$name"
      chmod 755 "$lib/$name"
      patchelf --set-rpath "\$ORIGIN" "$lib/$name"
      echo "$lib/$name" >> "$work/elf"
      echo "bundled $name <- $(readlink -f "$path")"
    fi
  done < "$work/deps"
  [ "$status" -eq 0 ] || exit 1
  while read -r f; do
    ldd "$f" | awk '$2 == "=>" && $1 !~ /\// { print $1, $3 }' | while read -r name path; do
      if echo "$name" | grep -Eq "$baseline"; then continue; fi
      case "$path" in
        "$dir"/*) ;;
        *) echo "FAIL  $f loads $name from $path"; exit 1 ;;
      esac
    done
  done < "$work/elf"
}

macho_deps() { # install names of the libraries file $1 loads
  otool -L "$1" | tail -n +2 | awk '{ print $1 }' | grep -vxF "$(otool -D "$1" | tail -n +2)" || true
}

macho_rpaths() {
  otool -l "$1" | awk '$1 == "cmd" { r = ($2 == "LC_RPATH") } r && $1 == "path" { print $2 }'
}

macos_bundle() {
  find "$dir/bin" "$dir/lib" -type f -exec sh -c 'file -b "$1" | grep -q Mach-O' _ {} \; -print > "$work/queue"
  cp "$work/queue" "$work/all"
  # Copied libraries are appended to the queue, so the loop also walks their
  # dependencies.
  n=1
  while f=$(sed -n "${n}p" "$work/queue"); [ -n "$f" ]; do
    n=$((n + 1))
    for dep in $(macho_deps "$f"); do
      case "$dep" in
        /usr/lib/* | /System/*) continue ;;
        @rpath/*)
          name=${dep#@rpath/}
          src=
          [ -e "$lib/$name" ] && continue
          for r in $(macho_rpaths "$f"); do
            case "$r" in /*) [ -e "$r/$name" ] && { src="$r/$name"; break; } ;; esac
          done ;;
        /*) name=$(basename "$dep"); src=$dep ;;
        *) echo "FAIL  $f loads $dep"; exit 1 ;;
      esac
      if [ ! -e "$lib/$name" ]; then
        [ -n "$src" ] || { echo "FAIL  $f: $dep not found"; exit 1; }
        cp -L "$src" "$lib/$name"
        chmod 755 "$lib/$name"
        install_name_tool -id "@rpath/$name" "$lib/$name"
        echo "$lib/$name" >> "$work/queue"
        echo "$lib/$name" >> "$work/all"
        echo "bundled $name <- $(cd "$(dirname "$src")" && pwd -P)/$(basename "$src")"
      fi
      [ "$dep" = "@rpath/$name" ] || install_name_tool -change "$dep" "@rpath/$name" "$f"
    done
  done
  while read -r f; do
    for r in $(macho_rpaths "$f"); do
      install_name_tool -delete_rpath "$r" "$f"
    done
    install_name_tool -add_rpath "@loader_path/$(rel_to_lib "$f")" "$f"
    codesign --force --sign - "$f"
  done < "$work/all"
  while read -r f; do
    for dep in $(macho_deps "$f"); do
      case "$dep" in
        /usr/lib/* | /System/*) ;;
        @rpath/*) [ -e "$lib/${dep#@rpath/}" ] || { echo "FAIL  $f loads missing $dep"; exit 1; } ;;
        *) echo "FAIL  $f loads $dep"; exit 1 ;;
      esac
    done
    codesign --verify --strict "$f"
  done < "$work/all"
}

case "$(uname -s)" in
  Linux) linux_bundle ;;
  Darwin) macos_bundle ;;
  *) echo "unsupported system: $(uname -s)"; exit 2 ;;
esac
