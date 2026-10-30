#!/usr/bin/env python3
"""Assemble changelog fragments (changelog.d/*.md) into CHANGELOG.md.

Each pull request adds one fragment instead of editing CHANGELOG.md, so open
pull requests never conflict on the changelog. See changelog.d/README.md for
the fragment format and RELEASING.md for when this runs.

Usage:
  changelog_assemble.py --version 5.0.0 [--date 2026-11-01] [--dry-run]
  changelog_assemble.py --check        # validate fragments only
"""

import argparse
import os
import re
import subprocess
import sys

REPO = "zorba-processor/zorba"
SECTIONS = ["Breaking changes", "Security", "Added", "Changed", "Fixed", "Removed"]
VERSION_RE = re.compile(r"^(\d+)\.(\d+)\.(\d+)(-[0-9A-Za-z.-]+)?$")
DATE_RE = re.compile(r"^\d{4}-\d{2}-\d{2}$")
MERGE_RE = re.compile(r"^Merge pull request #(\d+) from ")
SQUASH_RE = re.compile(r"\(#(\d+)\)\s*$")
LINK_RE = re.compile(r"\]\(https://github\.com/[^)]*/pull/\d+\)")


class FragmentError(Exception):
    pass


def parse_fragment(text, name="<fragment>"):
    """Return {section: [bullet, ...]}; a bullet is a list of lines."""
    result = {}
    section = None
    bullet = None
    for lineno, line in enumerate(text.splitlines(), 1):
        if not line.strip():
            bullet = None
            continue
        m = re.match(r"^###\s+(.*?)\s*$", line)
        if m:
            section = m.group(1)
            if section not in SECTIONS:
                raise FragmentError(
                    "%s:%d: unknown section '%s' (allowed: %s)"
                    % (name, lineno, section, ", ".join(SECTIONS)))
            result.setdefault(section, [])
            bullet = None
        elif line.startswith("- "):
            if section is None:
                raise FragmentError("%s:%d: bullet before any '### Section' heading" % (name, lineno))
            bullet = [line.rstrip()]
            result[section].append(bullet)
        elif line[0] in " \t" and bullet is not None:
            bullet.append(line.rstrip())
        elif line.lstrip().startswith("<!--"):
            continue
        else:
            raise FragmentError("%s:%d: expected '### Section' or '- ' bullet, got: %s" % (name, lineno, line))
    if not any(result.values()):
        raise FragmentError("%s: no entries" % name)
    return result


def pr_number_from_subjects(subjects):
    """Pick the PR number from commit subjects, oldest commit last."""
    for subject in reversed(subjects):
        m = MERGE_RE.match(subject) or SQUASH_RE.search(subject)
        if m:
            return int(m.group(1))
    return None


def git_pr_number(path, root):
    try:
        out = subprocess.run(
            ["git", "log", "--first-parent", "-m", "--diff-filter=A", "--format=%s", "--", path],
            cwd=root, capture_output=True, text=True, check=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return None
    return pr_number_from_subjects([s for s in out.splitlines() if s])


def add_link(bullet, pr, repo=REPO):
    lines = list(bullet)
    if pr is not None and not LINK_RE.search(lines[-1]):
        lines[-1] += " ([#%d](https://github.com/%s/pull/%d))" % (pr, repo, pr)
    return lines


def _find_release(lines, base):
    for i, line in enumerate(lines):
        if re.match(r"^## \[%s\]" % re.escape(base), line):
            return i
    return None


def _section_end(lines, start):
    for j in range(start + 1, len(lines)):
        if lines[j].startswith("## ") or lines[j].startswith("### "):
            return j
    return len(lines)


def insert_entries(changelog, base, entries, date=None):
    """entries: {section: [bullet lines list, ...]} already in final order."""
    lines = changelog.split("\n")
    rel = _find_release(lines, base)
    if rel is None:
        # New release section: after '## [Unreleased]', before the first other '## '.
        pos = len(lines)
        for i, line in enumerate(lines):
            if line.startswith("## ") and not line.startswith("## [Unreleased]"):
                pos = i
                break
        heading = "## [%s] - %s" % (base, date or "Unreleased")
        lines[pos:pos] = [heading, ""]
        rel = pos
    elif date:
        lines[rel] = "## [%s] - %s" % (base, date)
    for section in SECTIONS:
        bullets = entries.get(section)
        if not bullets:
            continue
        rel_end = next((j for j in range(rel + 1, len(lines)) if lines[j].startswith("## ")), len(lines))
        sec = None
        for j in range(rel + 1, rel_end):
            if lines[j] == "### " + section:
                sec = j
                break
        flat = [l for b in bullets for l in b]
        if sec is None:
            # Insert before the first existing known section that follows in order.
            later = SECTIONS[SECTIONS.index(section) + 1:]
            pos = rel_end
            for j in range(rel + 1, rel_end):
                if lines[j].startswith("### ") and lines[j][4:] in later:
                    pos = j
                    break
            if pos == rel_end:
                while pos > rel + 1 and not lines[pos - 1].strip():
                    pos -= 1
                lines[pos:pos] = [""] + ["### " + section, ""] + flat
            else:
                lines[pos:pos] = ["### " + section, ""] + flat + [""]
        else:
            end = _section_end(lines, sec)
            while end > sec + 1 and not lines[end - 1].strip():
                end -= 1
            lines[end:end] = flat
    return "\n".join(lines)


def list_fragments(directory):
    if not os.path.isdir(directory):
        return []
    return sorted(
        os.path.join(directory, f) for f in os.listdir(directory)
        if f.endswith(".md") and f.lower() != "readme.md")


def assemble(root, base, date=None, repo=REPO, pr_lookup=None, changelog_path="CHANGELOG.md",
             fragments_dir="changelog.d"):
    """Return (new_changelog_text, [fragment paths]) without touching the disk."""
    pr_lookup = pr_lookup or git_pr_number
    frag_dir = os.path.join(root, fragments_dir)
    items = []
    for path in list_fragments(frag_dir):
        rel = os.path.relpath(path, root)
        with open(path, encoding="utf-8") as f:
            parsed = parse_fragment(f.read(), rel)
        items.append((pr_lookup(rel, root), os.path.basename(path), path, parsed))
    # PR number order; fragments without a PR number come last, by name.
    items.sort(key=lambda it: (it[0] is None, it[0] or 0, it[1]))
    entries = {s: [] for s in SECTIONS}
    for pr, _name, _path, parsed in items:
        for section, bullets in parsed.items():
            for b in bullets:
                entries[section].append(add_link(b, pr, repo))
    with open(os.path.join(root, changelog_path), encoding="utf-8") as f:
        text = f.read()
    return insert_entries(text, base, entries, date), [it[2] for it in items]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", help="target release X.Y.Z[-pre]; a prerelease uses the X.Y.Z section")
    ap.add_argument("--date", help="YYYY-MM-DD; sets the section date (default: keep the heading, new sections say Unreleased)")
    ap.add_argument("--dry-run", action="store_true", help="print the resulting CHANGELOG.md, change nothing")
    ap.add_argument("--check", action="store_true", help="only validate the fragments")
    ap.add_argument("--root", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."),
                    help="repository root (default: parent of scripts/)")
    ap.add_argument("--repo", default=REPO, help="GitHub owner/name used for PR links")
    args = ap.parse_args(argv)
    root = os.path.abspath(args.root)

    if args.check:
        bad = 0
        for path in list_fragments(os.path.join(root, "changelog.d")):
            try:
                with open(path, encoding="utf-8") as f:
                    parse_fragment(f.read(), os.path.relpath(path, root))
            except FragmentError as e:
                print("error:", e, file=sys.stderr)
                bad += 1
        return 1 if bad else 0

    if not args.version:
        ap.error("--version is required")
    m = VERSION_RE.match(args.version)
    if not m:
        ap.error("--version must be X.Y.Z or X.Y.Z-pre")
    if args.date and not DATE_RE.match(args.date):
        ap.error("--date must be YYYY-MM-DD")
    base = "%s.%s.%s" % m.groups()[:3]

    try:
        text, frags = assemble(root, base, args.date, args.repo)
    except FragmentError as e:
        print("error:", e, file=sys.stderr)
        return 1
    if args.dry_run:
        sys.stdout.write(text)
        print("%d fragment(s) would be assembled" % len(frags), file=sys.stderr)
        return 0
    if not frags and not args.date:
        print("No fragments to assemble.", file=sys.stderr)
        return 0
    with open(os.path.join(root, "CHANGELOG.md"), "w", encoding="utf-8") as f:
        f.write(text)
    for p in frags:
        os.remove(p)
    print("Assembled %d fragment(s) into CHANGELOG.md [%s]" % (len(frags), base), file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
