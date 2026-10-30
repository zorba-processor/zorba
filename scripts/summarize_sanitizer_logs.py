#!/usr/bin/env python3
"""Summarizes AddressSanitizer / LeakSanitizer / UBSan log files.

usage: summarize_sanitizer_logs.py <log-dir> <source-root> [max-rows]

Groups reports by kind and by the first stack frame inside the source tree,
and prints a Markdown report (suitable for $GITHUB_STEP_SUMMARY).
"""
import collections
import os
import re
import sys

FRAME = re.compile(r'^\s*#\d+ 0x[0-9a-f]+ in (.+?) (/\S+?):(\d+)')
LEAK = re.compile(r'^(Direct|Indirect) leak of (\d+) byte')
ASAN_ERROR = re.compile(r'ERROR: AddressSanitizer: ([\w-]+)')
UBSAN = re.compile(r'^(/\S+?):(\d+):\d+: runtime error: (.*)$')


def first_source_frame(lines, start, root):
    for line in lines[start:]:
        if not line.strip():
            break
        m = FRAME.match(line)
        if m and m.group(2).startswith(root):
            path = os.path.relpath(m.group(2), root)
            return f'{path}:{m.group(3)} ({m.group(1)[:80]})'
    return '(no frame in source tree)'


def main():
    log_dir, root = sys.argv[1], os.path.abspath(sys.argv[2])
    max_rows = int(sys.argv[3]) if len(sys.argv) > 3 else 40
    leaks = collections.defaultdict(lambda: [0, 0])   # key -> [reports, bytes]
    errors = collections.Counter()
    ubsan = collections.Counter()
    files = 0

    for name in sorted(os.listdir(log_dir)) if os.path.isdir(log_dir) else []:
        files += 1
        with open(os.path.join(log_dir, name), errors='replace') as f:
            lines = f.read().splitlines()
        for i, line in enumerate(lines):
            m = LEAK.match(line)
            if m:
                key = (m.group(1), first_source_frame(lines, i + 1, root))
                leaks[key][0] += 1
                leaks[key][1] += int(m.group(2))
                continue
            m = ASAN_ERROR.search(line)
            if m:
                errors[(m.group(1), first_source_frame(lines, i + 1, root))] += 1
                continue
            m = UBSAN.match(line)
            if m:
                path = m.group(1)
                if path.startswith(root):
                    path = os.path.relpath(path, root)
                ubsan[(f'{path}:{m.group(2)}', m.group(3)[:100])] += 1

    print('# Sanitizer report\n')
    print(f'{files} log files; {len(errors)} distinct memory errors, '
          f'{len(ubsan)} distinct UB sites, {len(leaks)} distinct leak sites.\n')

    print('## AddressSanitizer errors\n')
    print('| Count | Kind | First frame in source |\n|---:|---|---|')
    for (kind, where), n in errors.most_common(max_rows):
        print(f'| {n} | {kind} | `{where}` |')

    print('\n## Undefined behavior\n')
    print('| Count | Location | Message |\n|---:|---|---|')
    for (where, msg), n in ubsan.most_common(max_rows):
        print(f'| {n} | `{where}` | {msg} |')

    print('\n## Leaks\n')
    print('| Reports | Bytes | Kind | First frame in source |\n|---:|---:|---|---|')
    ranked = sorted(leaks.items(), key=lambda kv: (-kv[1][0], -kv[1][1]))
    for (kind, where), (n, size) in ranked[:max_rows]:
        print(f'| {n} | {size} | {kind} | `{where}` |')


if __name__ == '__main__':
    main()
