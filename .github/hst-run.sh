#!/bin/sh
set -u
git config --global --add safe.directory '*'
cmake --preset full-release > /dev/null
cmake --build --preset full-release > out/build.log 2>&1 || { grep -n "error" out/build.log | head -40; tail -30 out/build.log; exit 1; }
ctest --test-dir build/full-release -j"$(nproc)" -R "dynamic-calls|hoist_|dyncall|HigherOrder|hof|jsoniq|json|iterators|zorba/optim|zorba/scripting|zorba/windowing|window_memory|zorba/index" --output-junit /src/out/junit.xml > out/ctest.log 2>&1
grep -E "tests passed|tests failed|\(Failed\)|\(Timeout\)|\(SEGFAULT\)|\(Subprocess" out/ctest.log | cut -c1-120
cp build/full-release/Testing/Temporary/LastTest.log out/ 2>/dev/null
cp -r bench/dynamic-calls /tmp/q
BASE=$(git merge-base HEAD origin/master)
git worktree add -q /tmp/base "$BASE"
(cd /tmp/base && cmake --preset full-release > /dev/null && cmake --build --preset full-release > /src/out/build-base.log 2>&1) || { tail -30 out/build-base.log; exit 1; }
ZH=$(find build/full-release/bin -maxdepth 1 -name zorba -type f | head -n 1)
ZB=$(find /tmp/base/build/full-release/bin -maxdepth 1 -name zorba -type f | head -n 1)
echo "base $BASE head $(git rev-parse --short HEAD~1)"
python3 .github/hst-bench.py "$ZB" "$ZH"
echo MUTATION
python3 .github/hst-mut.py
cmake --build --preset full-release > out/build-mut.log 2>&1 || { tail -20 out/build-mut.log; exit 1; }
ctest --test-dir build/full-release -j"$(nproc)" -R "dynamic-calls/e1[346]" > out/ctest-mut.log 2>&1
grep -E "MUT|tests passed|tests failed|\(Failed\)|e1[346]-" out/ctest-mut.log | cut -c1-110
exit 0
