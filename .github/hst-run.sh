#!/bin/sh
set -u
git config --global --add safe.directory '*'
cmake --preset full-release > /dev/null
cmake --build --preset full-release > out/build.log 2>&1 || { grep -n "error" out/build.log | head -40; tail -30 out/build.log; exit 1; }
cp -r bench/dynamic-calls /tmp/q
BASE=$(git merge-base HEAD origin/master)
git worktree add -q /tmp/base "$BASE"
(cd /tmp/base && cmake --preset full-release > /dev/null && cmake --build --preset full-release > /src/out/build-base.log 2>&1) || { tail -30 out/build-base.log; exit 1; }
ZH=$(find build/full-release/bin -maxdepth 1 -name zorba -type f | head -n 1)
ZB=$(find /tmp/base/build/full-release/bin -maxdepth 1 -name zorba -type f | head -n 1)
echo "base $BASE head $(git rev-parse --short HEAD~1)"
python3 .github/hst-bench.py "$ZB" "$ZH"
exit 0
