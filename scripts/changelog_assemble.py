#!/usr/bin/env python3
"""Assemble changelog fragments (changelog.d/*.md) into CHANGELOG.md.

Each pull request adds one fragment instead of editing CHANGELOG.md, so open
pull requests never conflict on the changelog. See changelog.d/README.md for
the fragment format and RELEASING.md for when this runs.

Usage:
  changelog_assemble.py --version 5.0.0 [--date 2026-11-01] [--dry-run]
  changelog_assemble.py --check        # validate fragments only
  changelog_assemble.py --notes-since v5.0.0-alpha.2 --version 5.0.0-alpha.3 [--tag v5.0.0-alpha.3]
                                       # release notes: entries not in CHANGELOG.md at the ref
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


PR_RE = re.compile(r"/pull/(\d+)\)")


def section_entries(changelog, base):
    """Return [(subsection heading, [bullet, ...])] of the [base] section.

    A bullet is a list of lines: a top-level '- ' line plus its indented
    continuation lines. Subsections without bullets (for example the "At a
    glance" table) are returned with an empty bullet list. A missing section
    gives [].
    """
    lines = changelog.split("\n")
    start = _find_release(lines, base)
    if start is None:
        return []
    result = []
    bullet = None
    pending_blank = []
    for line in lines[start + 1:]:
        if line.startswith("## "):
            break
        if line.startswith("### "):
            result.append((line[4:].strip(), []))
            bullet = None
            pending_blank = []
        elif line.startswith("- ") and result:
            bullet = [line.rstrip()]
            result[-1][1].append(bullet)
            pending_blank = []
        elif not line.strip():
            pending_blank.append(line)
        elif line[0] in " \t" and bullet is not None:
            bullet.extend(pending_blank)
            bullet.append(line.rstrip())
            pending_blank = []
        else:
            bullet = None
            pending_blank = []
    return result


def _entry_prs(bullet):
    return {int(n) for n in PR_RE.findall(" ".join(bullet))}


def _entry_text_key(bullet):
    return " ".join(" ".join(bullet).split()).lower()


def new_entries(old_changelog, new_changelog, base):
    """Entries of the [base] section of new_changelog that are not in old_changelog.

    Identity is the pull request number in the entry's link: an entry is old
    when it shares a pull request number with an old entry, so an entry edited
    between the two versions is not new. Entries without a link are compared by
    normalized text. Returns [(subsection, [bullet, ...])] in the current order,
    only subsections with new bullets.
    """
    old = [b for _h, bullets in section_entries(old_changelog, base) for b in bullets]
    old_prs = set()
    old_texts = set()
    for b in old:
        old_prs |= _entry_prs(b)
        old_texts.add(_entry_text_key(b))
    result = []
    for heading, bullets in section_entries(new_changelog, base):
        fresh = []
        for b in bullets:
            prs = _entry_prs(b)
            is_old = bool(prs & old_prs) if prs else _entry_text_key(b) in old_texts
            if not is_old:
                fresh.append(b)
        if fresh:
            result.append((heading, fresh))
    return result


def notes_since(old_changelog, new_changelog, base, prev_version, tag, repo=REPO):
    """Release notes text listing what changed in [base] since prev_version."""
    out = [
        "Prerelease of Zorba %s. These notes list what changed since %s. The full list of "
        "changes in %s so far, including the \"At a glance\" table, is in "
        "[CHANGELOG.md](https://github.com/%s/blob/%s/CHANGELOG.md)."
        % (base, prev_version, base, repo, tag),
        "",
        "## Changes since %s" % prev_version,
    ]
    for heading, bullets in new_entries(old_changelog, new_changelog, base):
        out += ["", "### " + heading, ""]
        out += [l for b in bullets for l in b]
    return "\n".join(out) + "\n"


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
    ap.add_argument("--notes-since", metavar="REF",
                    help="print release notes for --version: the entries of the [X.Y.Z] section of CHANGELOG.md "
                         "that are not in CHANGELOG.md at git REF (a tag such as v5.0.0-alpha.2)")
    ap.add_argument("--tag", help="tag the notes link to (default: v<version>)")
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

    if args.notes_since:
        try:
            old = subprocess.run(["git", "show", "%s:CHANGELOG.md" % args.notes_since], cwd=root,
                                 capture_output=True, text=True, check=True).stdout
        except (OSError, subprocess.CalledProcessError) as e:
            print("error: cannot read CHANGELOG.md at %s: %s" % (args.notes_since, getattr(e, "stderr", e)),
                  file=sys.stderr)
            return 1
        with open(os.path.join(root, "CHANGELOG.md"), encoding="utf-8") as f:
            new = f.read()
        prev = args.notes_since[1:] if args.notes_since.startswith("v") else args.notes_since
        sys.stdout.write(notes_since(old, new, base, prev, args.tag or "v" + args.version, args.repo))
        return 0

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
