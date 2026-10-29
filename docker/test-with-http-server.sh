#!/bin/sh
# Runs the CTest suite in the builder image with a local replacement for the
# retired zorbatest.28.io:8080 test server.
# usage: docker/test-with-http-server.sh <builder-tag> <build-dir-in-volume> [ctest args]
set -e
img=${1:-latest}; dir=${2:-full}; shift 2 || true
root=$(cd "$(dirname "$0")/.." && pwd)
docker build -q -t zorba-http-test -f "$root/docker/http-test-server/Dockerfile" "$root" >/dev/null
docker network create zorba-test >/dev/null 2>&1 || true
docker rm -f zorbatest >/dev/null 2>&1 || true
docker run -d --name zorbatest --network zorba-test --network-alias zorbatest.28.io zorba-http-test >/dev/null
trap 'docker rm -f zorbatest >/dev/null 2>&1' EXIT
docker run --rm --network zorba-test -v "$root":/src -v zorba-build:/build -v zorba-ccache:/ccache \
  "zorba-builder:$img" sh -c "cd /build/$dir && ctest -j\$(nproc) --timeout 120 $*"
