#!/bin/sh
# Build ICU, libxml2 and Xerces-C from the patched zorba-processor forks into
# <prefix>, for the macOS release binaries. Same tags and commits as
# docker/builder, docker/builder-latest and docker/runtime (keep the four in
# sync); see doc/KNOWN_ISSUES.txt for what the forks fix.
# Needs git, make, meson, ninja, cmake and pkg-config.
# Usage: build-macos-deps.sh <prefix>
set -eu
prefix=${1:?usage: build-macos-deps.sh <prefix>}
mkdir -p "$prefix"
prefix=$(cd "$prefix" && pwd)
src=$(mktemp -d)
trap 'rm -rf "$src"' EXIT
jobs=$(sysctl -n hw.ncpu)

clone() { # repo tag commit dir [git clone options]
  repo=$1 tag=$2 commit=$3 to=$4
  shift 4
  git clone -q --depth 1 "$@" --branch "$tag" "$repo" "$to"
  test "$(git -C "$to" rev-parse HEAD)" = "$commit"
}

clone https://github.com/zorba-processor/icu.git release-78.3-zorba.1 \
  2ed410f1d6bf3ff218985389538ddad641979af3 "$src/icu" --filter=blob:none --sparse
git -C "$src/icu" sparse-checkout set icu4c
(cd "$src/icu/icu4c/source" \
  && ./runConfigureICU MacOSX --prefix="$prefix" --enable-rpath --disable-samples --disable-tests \
  && make -j"$jobs" \
  && make install)

clone https://github.com/zorba-processor/libxml2.git v2.15.4-zorba.1 \
  e66d697e3a9a855084f3d9243bbb0a8695994cf2 "$src/libxml2"
PKG_CONFIG_PATH="$prefix/lib/pkgconfig" meson setup "$src/libxml2/build" "$src/libxml2" \
  --prefix="$prefix" --libdir=lib --buildtype=release \
  -Ddocs=disabled -Dpython=disabled -Dreadline=disabled -Dhistory=disabled \
  -Dicu=enabled -Dlegacy=enabled
meson install -C "$src/libxml2/build"

clone https://github.com/zorba-processor/xerces-c.git v3.3.0-zorba.5 \
  c35c28f192a28d9ae1ddde635294f43536dccffd "$src/xerces"
cmake -S "$src/xerces" -B "$src/xerces/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$prefix" -DCMAKE_INSTALL_LIBDIR=lib \
  -DCMAKE_PREFIX_PATH="$prefix" -DICU_ROOT="$prefix" \
  -Dtranscoder=icu -Dnetwork-accessor=curl
cmake --build "$src/xerces/build" --target xerces-c
cmake --install "$src/xerces/build/src"
