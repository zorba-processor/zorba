#!/usr/bin/env python3
"""Summarizes AddressSanitizer / LeakSanitizer / UBSan reports.

usage: summarize_sanitizer_logs.py <log-dir> <source-root> [max-rows]
                                   [--junit <ctest-junit.xml>]

Reads the sanitizer log files in <log-dir> and, with --junit, the test output
recorded by `ctest --output-junit`. The latter is needed for UBSan: with GCC,
-fsanitize=address,undefined links libasan and libubsan separately, and
libubsan then ignores log_path and prints its reports to stderr, so they only
end up in the test output.

Groups memory errors and leaks by kind and by the first stack frame inside the
source tree, and UB reports by site (the source location in the report), and
prints a Markdown report (suitable for $GITHUB_STEP_SUMMARY).
"""
import argparse
import collections
import os
import re
import xml.etree.ElementTree as ET

FRAME = re.compile(r'^\s*#\d+ 0x[0-9a-f]+ in (.+?) (/\S+?):(\d+)')
LEAK = re.compile(r'^(Direct|Indirect) leak of (\d+) byte')
ASAN_ERROR = re.compile(r'ERROR: AddressSanitizer: ([\w-]+)')
UBSAN = re.compile(r'(/\S+?):(\d+):\d+: runtime error: (.*)$')
NUMBER = re.compile(r'\b-?(0x[0-9a-fA-F]+|\d+)\b')


def first_source_frame(lines, start, root):
    for line in lines[start:]:
        if not line.strip():
            break
        m = FRAME.match(line)
        if m and m.group(2).startswith(root):
            path = os.path.relpath(m.group(2), root)
            return f'{path}:{m.group(3)} ({m.group(1)[:80]})'
    return '(no frame in source tree)'


class Report:
    def __init__(self, root):
        self.root = root
        self.leaks = collections.defaultdict(lambda: [0, 0])  # [reports, bytes]
        self.errors = collections.Counter()
        # site -> [reports, set of tests, Counter of messages]
        self.ub = collections.defaultdict(
            lambda: [0, set(), collections.Counter()])

    def scan(self, lines, source):
        root = self.root
        for i, line in enumerate(lines):
            m = LEAK.match(line)
            if m:
                key = (m.group(1), first_source_frame(lines, i + 1, root))
                self.leaks[key][0] += 1
                self.leaks[key][1] += int(m.group(2))
                continue
            m = ASAN_ERROR.search(line)
            if m:
                where = first_source_frame(lines, i + 1, root)
                self.errors[(m.group(1), where)] += 1
                continue
            m = UBSAN.search(line)
            if m:
                path = m.group(1)
                if path.startswith(root):
                    path = os.path.relpath(path, root)
                site = self.ub[f'{path}:{m.group(2)}']
                site[0] += 1
                site[1].add(source)
                # Values vary per report (e.g. the garbage loaded from an
                # uninitialized enum); group messages without them.
                site[2][NUMBER.sub('N', m.group(3))[:100]] += 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('log_dir')
    ap.add_argument('root')
    ap.add_argument('max_rows', nargs='?', type=int, default=40)
    ap.add_argument('--junit', help='ctest --output-junit file')
    args = ap.parse_args()

    report = Report(os.path.abspath(args.root))
    files = 0
    log_dir = args.log_dir
    for name in sorted(os.listdir(log_dir)) if os.path.isdir(log_dir) else []:
        files += 1
        with open(os.path.join(log_dir, name), errors='replace') as f:
            report.scan(f.read().splitlines(), name)

    tests = 0
    if args.junit and os.path.isfile(args.junit):
        for case in ET.parse(args.junit).getroot().iter('testcase'):
            tests += 1
            out = case.find('system-out')
            if out is not None and out.text:
                report.scan(out.text.splitlines(), case.get('name'))

    ub_reports = sum(s[0] for s in report.ub.values())
    print('# Sanitizer report\n')
    print(f'{files} log files, {tests} test outputs; '
          f'{len(report.errors)} distinct memory errors, '
          f'{len(report.ub)} UB sites ({ub_reports} reports), '
          f'{len(report.leaks)} distinct leak sites.\n')

    print('## AddressSanitizer errors\n')
    print('| Count | Kind | First frame in source |\n|---:|---|---|')
    for (kind, where), n in report.errors.most_common(args.max_rows):
        print(f'| {n} | {kind} | `{where}` |')

    print('\n## Undefined behavior (by site)\n')
    print('| Reports | Tests | Site | Message | Example test |\n'
          '|---:|---:|---|---|---|')
    ranked = sorted(report.ub.items(), key=lambda kv: (-kv[1][0], kv[0]))
    for where, (n, srcs, msgs) in ranked[:args.max_rows]:
        msg = msgs.most_common(1)[0][0]
        print(f'| {n} | {len(srcs)} | `{where}` | {msg} | {min(srcs)} |')

    print('\n## Leaks\n')
    print('| Reports | Bytes | Kind | First frame in source |\n|---:|---:|---|---|')
    ranked = sorted(report.leaks.items(), key=lambda kv: (-kv[1][0], -kv[1][1]))
    for (kind, where), (n, size) in ranked[:args.max_rows]:
        print(f'| {n} | {size} | {kind} | `{where}` |')


if __name__ == '__main__':
    main()
