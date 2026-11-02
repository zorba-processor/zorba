#!/usr/bin/env python3
"""Check that the two independent ground truths agree.

For every task, runs queries/<task>.jq with `zorba --directory <snapshot>`
and grades its result against the Python reference (reference.py) with the
task's own grading rule. Also checks that tasks/ground_truth.json is up to
date. Exits non-zero on any disagreement.

Usage: check_truth.py --zorba PATH [--snapshot DIR] [--timeout SECONDS]
"""

import argparse
import json
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import grading  # noqa: E402
import reference  # noqa: E402


def load_tasks():
    with open(os.path.join(HERE, "tasks", "tasks.json"), encoding="utf-8") as f:
        return json.load(f)["tasks"]


def run_zorba(zorba, snapshot, query, timeout):
    proc = subprocess.run(
        [zorba, "--directory", snapshot, "-f", "-q", query],
        capture_output=True, text=True, timeout=timeout)
    if proc.returncode != 0:
        raise RuntimeError(f"zorba exited with {proc.returncode}:\n"
                           f"{proc.stderr.strip()}\n{proc.stdout.strip()}")
    items = [json.loads(line) for line in proc.stdout.splitlines()
             if line.strip()]
    if len(items) != 1:
        raise RuntimeError(f"expected one JSON item, got {len(items)}: "
                           f"{proc.stdout[:500]}")
    return items[0]


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--zorba", required=True)
    ap.add_argument("--snapshot", default=reference.DEFAULT_SNAPSHOT)
    ap.add_argument("--timeout", type=float, default=600)
    args = ap.parse_args()

    truth = reference.compute(args.snapshot)
    with open(reference.TRUTH, encoding="utf-8") as f:
        committed = json.load(f)
    failures = []
    if committed != json.loads(json.dumps(truth)):
        failures.append("tasks/ground_truth.json is stale: run "
                        "reference.py --write")

    tasks = load_tasks()
    if sorted(t["id"] for t in tasks) != sorted(truth):
        failures.append("tasks.json and reference.py define different tasks")
    for task in tasks:
        tid = task["id"]
        query = os.path.join(HERE, "queries", tid + ".jq")
        start = time.monotonic()
        try:
            got = run_zorba(args.zorba, args.snapshot, query, args.timeout)
            why = grading.grade(got, truth[tid], task["grading"])
        except Exception as e:  # noqa: BLE001 - reported per task
            got, why = None, f"{type(e).__name__}: {e}"
        secs = time.monotonic() - start
        status = "ok  " if why is None else "FAIL"
        print(f"{status} {tid} ({secs:.1f} s)")
        if why is not None:
            print(f"     reference: {json.dumps(truth[tid])[:400]}")
            print(f"     zorba:     {json.dumps(got)[:400]}")
            print(f"     reason:    {why}")
            failures.append(tid)
    if failures:
        print(f"\n{len(failures)} failure(s): {', '.join(failures)}")
        return 1
    print(f"\nall {len(tasks)} tasks: zorba queries agree with reference.py")
    return 0


if __name__ == "__main__":
    sys.exit(main())
