#!/usr/bin/env python3
# Copyright 2006-2016 zorba.io
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Run (a shard of) the W3C QT3 test suite with the Zorba FOTS driver.

Each test set is run by one zorba process (test/fots_driver/cli.xq,
mode run-test-set). The driver evaluates the test cases in-process, so a
crash or hang takes the whole set down: when that happens, or the set's
output cannot be parsed, every test case of the set is re-run in its own
process with a per-test timeout, and the crashing/hanging ones are recorded
as errors.

Output: <out>/results.tsv with one line per test case:
  test-set <TAB> test-case <TAB> status <TAB> detail
status is one of: pass, fail, wrongError, n/a, skip, error.
"""

import argparse
import concurrent.futures
import os
import shutil
import subprocess
import sys
import tempfile
import time
import xml.etree.ElementTree as ET

FOTS = "{http://www.w3.org/2010/09/qt-fots-catalog}"


def load_catalog(catalog):
    """Returns [(set name, [case names])] in catalog order."""
    root = ET.parse(catalog).getroot()
    base = os.path.dirname(catalog)
    sets = []
    for ts in root.findall(FOTS + "test-set"):
        doc = ET.parse(os.path.join(base, ts.get("file"))).getroot()
        sets.append((ts.get("name"),
                     [tc.get("name") for tc in doc.findall(FOTS + "test-case")]))
    return sets


def shard_sets(sets, index, count):
    """Greedy bin packing by number of test cases (deterministic)."""
    bins = [[0, []] for _ in range(count)]
    for name, cases in sorted(sets, key=lambda s: (-len(s[1]), s[0])):
        b = min(bins, key=lambda x: x[0])
        b[0] += max(len(cases), 1)
        b[1].append(name)
    return set(bins[index][1])


def classify(tc):
    """Maps one <fots:test-case result=...> of the driver output to
    (status, detail)."""
    result = tc.get("result", "")
    comment = tc.get("comment", "") or tc.get("info", "")
    if result == "pass":
        # EXPECTED_FOTS_FAILURE SLOW cases are reported as 'pass' without
        # being run.
        if "was not run" in comment:
            return "skip", "EXPECTED_FOTS_FAILURE SLOW (not run)"
        return "pass", ""
    if result == "wrongError":
        return "wrongError", comment
    if result == "fail":
        # verbose driver output carries the expected/actual mismatch.
        errors = " ".join("".join(e.itertext()) for e in tc.iter(FOTS + "errors"))
        return "fail", errors or comment
    if result == "n/a":
        return "n/a", comment
    if result in ("tooBig", "notRun"):
        return "skip", "%s: %s" % (result, comment)
    if result == "disputed":
        return "skip", "disputed: %s" % comment
    return "error", "unknown driver result '%s'" % result


def one_line(s, limit=300):
    s = " ".join(str(s).split())
    return s[:limit]


class Runner:
    def __init__(self, args):
        self.args = args

    def cmd(self, out_file, mode, test_set, test_case=None, verbose=False):
        a = self.args
        c = [a.zorba, "-f", "-q", a.driver,
             "-e", "fotsPath:=" + a.catalog,
             "-e", "fotsZorbaManifestPath:=" + a.manifest,
             "-e", "expectedFailuresPath:=" + a.expected_failures,
             "-e", "mode:=" + mode,
             "-e", "testSetName:=" + test_set,
             "-e", "verbose:=" + ("true" if verbose else "false"),
             "-e", "ctestMode:=false",
             "-e", "usePlanSerializer:=false",
             "--disable-http-resolution",
             "-o", out_file]
        if test_case is not None:
            c[c.index("-o"):c.index("-o")] = ["-e", "testCaseName:=" + test_case]
        return c

    def invoke(self, mode, test_set, test_case, timeout, verbose):
        """Runs the driver; returns (parsed test-case elements or None,
        error description, raw output path)."""
        fd, out = tempfile.mkstemp(suffix=".xml", dir=self.args.tmp)
        os.close(fd)
        workdir = tempfile.mkdtemp(dir=self.args.tmp)
        try:
            p = subprocess.run(self.cmd(out, mode, test_set, test_case, verbose),
                               cwd=workdir, stdin=subprocess.DEVNULL,
                               stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                               timeout=timeout)
        except subprocess.TimeoutExpired:
            return None, "timeout after %ds" % timeout, out
        finally:
            shutil.rmtree(workdir, ignore_errors=True)
        if p.returncode != 0:
            err = p.stderr.decode("utf-8", "replace").strip().splitlines()
            tail = one_line(" | ".join(err[-3:])) if err else ""
            if p.returncode < 0:
                return None, "crashed (signal %d) %s" % (-p.returncode, tail), out
            return None, "zorba exit %d %s" % (p.returncode, tail), out
        try:
            root = ET.parse(out).getroot()
        except ET.ParseError as e:
            return None, "unparseable driver output: %s" % e, out
        return root.iter(FOTS + "test-case"), "", out

    def run_set(self, name, cases):
        a = self.args
        start = time.monotonic()
        results = {}
        timeout = min(a.set_timeout_max, a.set_timeout_base + 2 * len(cases))
        if time.monotonic() > a.deadline:
            return name, {c: ("error", "not run: shard deadline reached") for c in cases}, 0.0, "deadline"
        tcs, err, out = self.invoke("run-test-set", name, None, timeout, a.verbose)
        if tcs is not None:
            for tc in tcs:
                if tc.get("result") is not None and tc.get("name") in cases:
                    results[tc.get("name")] = classify(tc)
            self.keep(out, name)
            missing = [c for c in cases if c not in results]
            how = "set"
        else:
            os.unlink(out)
            missing = list(cases)
            how = "per-case (%s)" % err
        # Fallback: run the missing cases one by one.
        for c in missing:
            if time.monotonic() > a.deadline:
                results[c] = ("error", "not run: shard deadline reached")
                continue
            results[c] = self.run_case(name, c)
        return name, results, time.monotonic() - start, how

    def run_case(self, name, case):
        tcs, err, out = self.invoke("run-test-case", name, case,
                                    self.args.case_timeout, False)
        if os.path.exists(out):
            os.unlink(out)
        if tcs is None:
            return ("error", err)
        for tc in tcs:
            if tc.get("name") == case and tc.get("result") is not None:
                return classify(tc)
        return ("error", "test case missing from driver output")

    def keep(self, out, name):
        dest = os.path.join(self.args.out, "xml", name + ".xml")
        shutil.move(out, dest)
        # mkstemp creates 0600 files; the container runs as root and the
        # artifact upload as the runner user.
        os.chmod(dest, 0o644)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--zorba", required=True)
    ap.add_argument("--driver", required=True, help="test/fots_driver/cli.xq")
    ap.add_argument("--manifest", required=True, help="FOTSZorbaManifest.xml")
    ap.add_argument("--expected-failures", required=True,
                    help="FOTSExpectedFailures.xml generated by CMake")
    ap.add_argument("--catalog", required=True, help="qt3tests catalog.xml")
    ap.add_argument("--out", required=True)
    ap.add_argument("--shard", type=int, default=0)
    ap.add_argument("--shards", type=int, default=1)
    ap.add_argument("--filter", default="",
                    help="comma-separated test-set name prefixes")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 1)
    ap.add_argument("--case-timeout", type=int, default=60)
    ap.add_argument("--set-timeout-base", type=int, default=120,
                    help="test-set timeout: base + 2s per test case, capped")
    ap.add_argument("--set-timeout-max", type=int, default=1800)
    ap.add_argument("--budget", type=int, default=5 * 3600,
                    help="seconds after which no new test is started")
    ap.add_argument("--baseline", default="",
                    help="baseline pass list; baseline cases that do not pass "
                         "are re-run once on their own")
    ap.add_argument("--max-retries", type=int, default=300,
                    help="cap on the baseline re-runs (a real regression "
                         "storm should not double the run time)")
    ap.add_argument("--verbose", action="store_true",
                    help="keep the query and actual result of failing cases "
                         "in the per-set XML")
    a = ap.parse_args()
    a.deadline = time.monotonic() + a.budget
    for attr in ("zorba", "driver", "manifest", "expected_failures", "catalog", "out"):
        setattr(a, attr, os.path.abspath(getattr(a, attr)))
    os.makedirs(os.path.join(a.out, "xml"), exist_ok=True)
    a.tmp = tempfile.mkdtemp(prefix="qt3-")

    sets = load_catalog(a.catalog)
    if a.filter:
        prefixes = tuple(p for p in a.filter.split(",") if p)
        sets = [s for s in sets if s[0].startswith(prefixes)]
    mine = shard_sets(sets, a.shard, a.shards)
    sets = [s for s in sets if s[0] in mine]
    total = sum(len(c) for _, c in sets)
    print("shard %d/%d: %d test sets, %d test cases, %d jobs"
          % (a.shard, a.shards, len(sets), total, a.jobs), flush=True)

    runner = Runner(a)
    results = {}
    log = open(os.path.join(a.out, "sets.log"), "w")
    done = 0
    # Big sets first so that they do not end up alone at the tail.
    order = sorted(sets, key=lambda s: -len(s[1]))
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
        futs = [ex.submit(runner.run_set, n, c) for n, c in order]
        for f in concurrent.futures.as_completed(futs):
            name, res, secs, how = f.result()
            results[name] = res
            done += len(res)
            n = sum(1 for s, _ in res.values() if s == "pass")
            line = "%-45s %5d cases %5d pass %7.1fs %s" % (name, len(res), n, secs, how)
            print("[%d/%d] %s" % (done, total, line), flush=True)
            log.write(line + "\n")

    if a.baseline and os.path.exists(a.baseline):
        with open(a.baseline) as f:
            base = {l.strip() for l in f if l.strip() and not l.startswith("#")}
        retry = [(s, c) for s, _ in sets for c, (st, _) in results[s].items()
                 if "%s/%s" % (s, c) in base and st != "pass"]
        print("re-running %d baseline cases that did not pass" % len(retry), flush=True)
        for s, c in retry[:a.max_retries]:
            st, det = runner.run_case(s, c)
            if st == "pass":
                results[s][c] = ("pass", "flaky: passed on re-run (%s)"
                                 % one_line(results[s][c][1], 150))
            log.write("retry %s/%s -> %s\n" % (s, c, st))

    with open(os.path.join(a.out, "results.tsv"), "w") as f:
        for s, cases in sets:
            for c in cases:
                st, det = results[s].get(c, ("error", "no result"))
                f.write("%s\t%s\t%s\t%s\n" % (s, c, st, one_line(det)))
    log.close()


if __name__ == "__main__":
    sys.exit(main())
