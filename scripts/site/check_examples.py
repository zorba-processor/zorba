#!/usr/bin/env python3
"""Compare actual example outputs against the expected .out files.

Usage: check_examples.py <examples-dir> <actual-dir> [--record]

<actual-dir> is what run_examples.sh (and the C++ step) produced: for each
example <name>.out, <name>.err and <name>.rc. Without --record, any example
that exits non-zero, has no expected .out file, or whose output differs makes
the script exit with status 1. With --record nothing fails; the actual
outputs are left in <actual-dir> so they can be copied over the expected files.
"""
import difflib
import sys
from pathlib import Path

SOURCE_SUFFIXES = {".xq", ".jq", ".cpp"}


def normalize(text: str) -> str:
    lines = [line.rstrip() for line in text.replace("\r\n", "\n").split("\n")]
    return "\n".join(lines).strip("\n") + "\n"


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    record = "--record" in sys.argv
    examples, actual = Path(args[0]), Path(args[1])
    failures = 0
    total = 0
    for src in sorted(examples.rglob("*")):
        if src.suffix not in SOURCE_SUFFIXES or not src.is_file():
            continue
        rel = src.relative_to(examples).with_suffix("")
        total += 1
        out_file = actual / rel.with_suffix(".out")
        rc_file = actual / rel.with_suffix(".rc")
        err_file = actual / rel.with_suffix(".err")
        if not out_file.exists():
            print(f"MISSING RUN  {rel}")
            failures += 1
            continue
        got = normalize(out_file.read_text(encoding="utf-8"))
        rc = rc_file.read_text().strip() if rc_file.exists() else "0"
        err = err_file.read_text(encoding="utf-8") if err_file.exists() else ""
        if rc != "0":
            print(f"ERROR (exit {rc})  {rel}\n{err}")
            failures += 1
            continue
        expected_file = examples / rel.with_suffix(".out")
        if not expected_file.exists():
            print(f"NO EXPECTED  {rel}")
            failures += 1
            continue
        want = normalize(expected_file.read_text(encoding="utf-8"))
        if got != want:
            print(f"MISMATCH     {rel}")
            sys.stdout.writelines(
                difflib.unified_diff(
                    want.splitlines(True), got.splitlines(True),
                    "expected", "actual"))
            failures += 1
        else:
            print(f"ok           {rel}")
    print(f"\n{total} examples, {failures} problem(s)")
    if record:
        print("record mode: not failing")
        return 0
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
