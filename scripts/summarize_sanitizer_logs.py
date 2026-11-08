#!/usr/bin/env python3
"""Summarizes AddressSanitizer / LeakSanitizer / UBSan reports.

usage: summarize_sanitizer_logs.py <log-dir> <source-root> [max-rows]
                                   [--junit <ctest-junit.xml>]
       summarize_sanitizer_logs.py --baseline <file> --ctest-log <ctest.log>
                                   [--summary <file>] [--ctest-exit-code <file>]

Reads the sanitizer log files in <log-dir> and, with --junit, the test output
recorded by `ctest --output-junit`. The latter is needed for UBSan: with GCC,
-fsanitize=address,undefined links libasan and libubsan separately, and
libubsan then ignores log_path and prints its reports to stderr, so they only
end up in the test output.

Groups memory errors and leaks by kind and by the first stack frame inside the
source tree, and UB reports by site (the source location in the report), and
prints a Markdown report (suitable for $GITHUB_STEP_SUMMARY).

With --baseline and --ctest-log it instead compares the failed tests in a
ctest log with a baseline of known failures (test/baselines/sanitizers-*.txt)
and exits 1 on any failure that the baseline does not allow:
  - a test that fails and is not in the baseline;
  - a baseline test that fails with a different failure kind than recorded
    (ctest prints the kind: Failed, Subprocess aborted, Timeout, SEGFAULT, ...).
The comparison is refused (exit 1) when the log has no summary line, when the
number of listed failures differs from the summary's count, or when the exit
code in --ctest-exit-code does not fit the log (0 with failures; 8 without
failures; anything but 0 or 8).
A baseline test that passes (or no longer exists) is only a warning: it
should be removed from the baseline, but a fix that merges first must not turn
the job red. Baseline lines are `test name | kind | reason`; kind `flaky`
allows the test to pass or fail in any way. Lines starting with # and empty
lines are ignored.
"""
import argparse
import collections
import os
import re
import sys
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


# A name may contain spaces; ctest may append labels after the kind.
FAILED_TEST = re.compile(r'^\s*\d+ - (.+?) \(([^()]+)\)(?:\s+\S.*)?$')
FAILED_LIST_ENTRY = re.compile(r'^\s*\d+ - ')
SUMMARY = re.compile(r'^\s*(\d+)% tests passed(?:, (\d+) tests failed)? '
                     r'out of (\d+)\s*$')
RAN_TEST = re.compile(r'^\s*\d+/\d+ Test\s+#\d+: (.+?) \.{3,}')
CTEST_EXIT_FAILED_TESTS = 8
FLAKY = 'flaky'


def read_baseline(path):
    """Returns {test name: (kind, reason)}; raises ValueError on a bad line."""
    entries = {}
    with open(path) as f:
        for number, line in enumerate(f, 1):
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            fields = [x.strip() for x in line.split('|', 2)]
            if len(fields) != 3 or not all(fields):
                raise ValueError(f'{path}:{number}: expected '
                                 '"test name | kind | reason"')
            if fields[0] in entries:
                raise ValueError(f'{path}:{number}: duplicate {fields[0]}')
            entries[fields[0]] = (fields[1], fields[2])
    return entries


def read_ctest_log(path):
    """Returns (ran tests, {failed test: kind}, unparsed list entries,
    (failed, total) from the last summary line or None)."""
    ran, failed, unparsed, summary = set(), {}, [], None
    in_failed_list = False
    with open(path, errors='replace') as f:
        for line in f:
            line = line.rstrip('\n')
            m = SUMMARY.match(line)
            if m:
                summary = (int(m.group(2) or 0), int(m.group(3)))
            if line.startswith('The following tests FAILED'):
                in_failed_list = True
                continue
            if in_failed_list:
                m = FAILED_TEST.match(line)
                if m:
                    failed[m.group(1)] = m.group(2)
                    continue
                if FAILED_LIST_ENTRY.match(line):
                    unparsed.append(line.strip())
                    continue
                in_failed_list = False
            m = RAN_TEST.match(line)
            if m:
                ran.add(m.group(1))
    return ran, failed, unparsed, summary


def read_exit_code(path):
    try:
        with open(path) as f:
            return int(f.read().strip())
    except (OSError, ValueError):
        return None


def check_run(log_path, failed, unparsed, summary, exit_code_path):
    """Returns the errors that make the log untrustworthy."""
    if summary is None:
        return [f'no ctest summary line in {log_path}; the run did not '
                'finish, so it cannot be compared with the baseline']
    errors = []
    count = summary[0]
    if unparsed:
        errors.append(f'{len(unparsed)} line(s) of the failed-test list '
                      f'could not be parsed: {unparsed[0]}')
    if len(failed) + len(unparsed) != count:
        errors.append(f'ctest reports {count} failed test(s) but the log '
                      f'lists {len(failed) + len(unparsed)}')
    if exit_code_path:
        rc = read_exit_code(exit_code_path)
        if rc is None:
            errors.append(f'no ctest exit code in {exit_code_path}')
        elif rc == 0 and count:
            errors.append(f'ctest exited 0 but reports {count} failure(s)')
        elif rc == CTEST_EXIT_FAILED_TESTS and not count:
            errors.append(f'ctest exited {rc} (tests failed) but reports no '
                          'failure; a failed test is missing from the log')
        elif rc not in (0, CTEST_EXIT_FAILED_TESTS):
            errors.append(f'ctest exited {rc}, which is not "all passed" (0) '
                          f'or "tests failed" ({CTEST_EXIT_FAILED_TESTS})')
    return errors


def compare(baseline_path, log_path, summary_path, exit_code_path=None):
    baseline = read_baseline(baseline_path)
    ran, failed, unparsed, summary = read_ctest_log(log_path)
    errors = check_run(log_path, failed, unparsed, summary, exit_code_path)
    if errors:
        for e in errors:
            print(f'::error::{e}')
        if summary_path:
            with open(summary_path, 'a') as f:
                f.write('## :x: ctest result cannot be trusted\n\n'
                        + ''.join(f'- {e}\n' for e in errors))
        return 1

    new = sorted(t for t in failed if t not in baseline)
    changed = sorted(t for t, k in failed.items()
                     if t in baseline and baseline[t][0] != FLAKY
                     and baseline[t][0] != k)
    fixed = sorted(t for t, (k, _) in baseline.items()
                   if k != FLAKY and t in ran and t not in failed)
    gone = sorted(t for t, (k, _) in baseline.items()
                  if k != FLAKY and t not in ran)
    known = sorted(t for t in failed if t in baseline and t not in changed)

    md = [f'## Failures against {baseline_path}\n',
          f'{len(failed)} failed: {len(known)} known, {len(new)} new, '
          f'{len(changed)} with a different failure kind; '
          f'{len(fixed)} baseline tests pass, {len(gone)} were not run.\n']
    if new:
        md.append(f'### :x: {len(new)} new failure(s)\n')
        md += [f'- `{t}` ({failed[t]})' for t in new]
        md.append('')
    if changed:
        md.append(f'### :x: {len(changed)} baseline test(s) fail differently\n')
        md += [f'- `{t}`: baseline `{baseline[t][0]}`, now `{failed[t]}`'
               for t in changed]
        md.append('')
    if fixed or gone:
        md.append('### :warning: Remove from the baseline\n')
        md += [f'- `{t}` passes ({baseline[t][1]})' for t in fixed]
        md += [f'- `{t}` was not run ({baseline[t][1]})' for t in gone]
        md.append('')
    text = '\n'.join(md)
    print(text)
    if summary_path:
        with open(summary_path, 'a') as f:
            f.write(text + '\n')
    for t in new:
        print(f'::error::new failure not in {baseline_path}: {t} ({failed[t]})')
    for t in changed:
        print(f'::error::{t} fails with a different kind than {baseline_path} '
              f'records: was {baseline[t][0]}, now {failed[t]}')
    for t in fixed + gone:
        print(f'::warning::remove {t} from {baseline_path}: '
              + ('it passes' if t in fixed else 'it was not run'))
    return 1 if new or changed else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('log_dir', nargs='?')
    ap.add_argument('root', nargs='?')
    ap.add_argument('max_rows', nargs='?', type=int, default=40)
    ap.add_argument('--junit', help='ctest --output-junit file')
    ap.add_argument('--baseline', help='file of known failing tests')
    ap.add_argument('--ctest-log', help='ctest output to compare')
    ap.add_argument('--summary', help='file to append the comparison to')
    ap.add_argument('--ctest-exit-code', help='file with the ctest exit code')
    args = ap.parse_args()

    if args.baseline or args.ctest_log:
        if not (args.baseline and args.ctest_log):
            ap.error('--baseline and --ctest-log go together')
        try:
            sys.exit(compare(args.baseline, args.ctest_log, args.summary,
                             args.ctest_exit_code))
        except ValueError as e:
            print(f'::error::{e}')
            sys.exit(1)
    if not (args.log_dir and args.root):
        ap.error('log_dir and root are required')

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
