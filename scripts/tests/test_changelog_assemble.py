import contextlib
import io
import os
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import changelog_assemble as ca  # noqa: E402

BASE = """# Changelog

## [Unreleased]

## [5.0.0] - Unreleased

### At a glance

| a | b |

### Security

- Old security entry ([#1](https://github.com/zorba-processor/zorba/pull/1)).

### Fixed

- Old fix ([#2](https://github.com/zorba-processor/zorba/pull/2)).

### Removed

- Old removal.

## Before 5.0

Text.
"""


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


class ParseTest(unittest.TestCase):
    def test_basic_and_continuation(self):
        r = ca.parse_fragment("### Added\n\n- One\n  continued\n- Two\n\n### Fixed\n- Three\n")
        self.assertEqual(r["Added"], [["- One", "  continued"], ["- Two"]])
        self.assertEqual(r["Fixed"], [["- Three"]])

    def test_unknown_section(self):
        with self.assertRaises(ca.FragmentError):
            ca.parse_fragment("### Misc\n- x\n")

    def test_bullet_without_section(self):
        with self.assertRaises(ca.FragmentError):
            ca.parse_fragment("- x\n")

    def test_stray_text(self):
        with self.assertRaises(ca.FragmentError):
            ca.parse_fragment("### Added\nprose\n")

    def test_empty(self):
        with self.assertRaises(ca.FragmentError):
            ca.parse_fragment("### Added\n")


class PrNumberTest(unittest.TestCase):
    def test_merge(self):
        self.assertEqual(ca.pr_number_from_subjects(["Merge pull request #12 from zorba-processor/x"]), 12)

    def test_squash(self):
        self.assertEqual(ca.pr_number_from_subjects(["Fix thing (#34)"]), 34)

    def test_none(self):
        self.assertIsNone(ca.pr_number_from_subjects(["Direct commit"]))

    def test_oldest_wins(self):
        self.assertEqual(ca.pr_number_from_subjects(["Rename (#9)", "Add (#5)"]), 5)


class AssembleTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        write(os.path.join(self.root, "CHANGELOG.md"), BASE)
        write(os.path.join(self.root, "changelog.d", "README.md"), "docs\n")

    def tearDown(self):
        self.tmp.cleanup()

    def run_assemble(self, prs, base="5.0.0", date=None):
        return ca.assemble(self.root, base, date, pr_lookup=lambda rel, root: prs.get(os.path.basename(rel)))

    def test_inserts_in_order_and_links(self):
        write(os.path.join(self.root, "changelog.d", "b.md"), "### Added\n- B feature\n\n### Fixed\n- B fix\n")
        write(os.path.join(self.root, "changelog.d", "a.md"), "### Added\n- A feature\n")
        text, frags = self.run_assemble({"a.md": 20, "b.md": 10})
        self.assertEqual(len(frags), 2)
        link = "([#%d](https://github.com/zorba-processor/zorba/pull/%d))"
        self.assertLess(text.index("- B feature " + link % (10, 10)), text.index("- A feature " + link % (20, 20)))
        # Added is created between Security and Fixed.
        self.assertLess(text.index("### Security"), text.index("### Added"))
        self.assertLess(text.index("### Added"), text.index("### Fixed"))
        # Appended after the existing Fixed entry.
        self.assertLess(text.index("- Old fix"), text.index("- B fix"))
        self.assertLess(text.index("- B fix"), text.index("### Removed"))
        self.assertIn("\n## Before 5.0\n", text)

    def test_no_pr_no_link_and_sorted_last(self):
        write(os.path.join(self.root, "changelog.d", "x.md"), "### Removed\n- X\n")
        write(os.path.join(self.root, "changelog.d", "y.md"), "### Removed\n- Y\n")
        text, _ = self.run_assemble({"y.md": 3})
        self.assertIn("- Old removal.\n- Y ([#3]", text)
        self.assertIn("\n- X\n", text)
        self.assertLess(text.index("- Y ("), text.index("- X\n"))

    def test_existing_link_not_doubled(self):
        write(os.path.join(self.root, "changelog.d", "x.md"),
              "### Fixed\n- X ([#7](https://github.com/zorba-processor/zorba/pull/7))\n")
        text, _ = self.run_assemble({"x.md": 8})
        self.assertNotIn("#8", text)

    def test_continuation_gets_link_on_last_line(self):
        write(os.path.join(self.root, "changelog.d", "x.md"), "### Fixed\n- X\n  more\n")
        text, _ = self.run_assemble({"x.md": 8})
        self.assertIn("- X\n  more ([#8]", text)

    def test_date_and_new_section(self):
        write(os.path.join(self.root, "changelog.d", "x.md"), "### Added\n- X\n")
        text, _ = self.run_assemble({}, base="5.1.0", date="2026-12-01")
        self.assertIn("## [Unreleased]\n\n## [5.1.0] - 2026-12-01\n\n### Added\n\n- X\n", text)
        self.assertLess(text.index("## [5.1.0]"), text.index("## [5.0.0]"))

    def test_date_replaces_unreleased(self):
        write(os.path.join(self.root, "changelog.d", "x.md"), "### Added\n- X\n")
        text, _ = self.run_assemble({}, date="2026-12-01")
        self.assertIn("## [5.0.0] - 2026-12-01", text)
        self.assertNotIn("## [5.0.0] - Unreleased", text)

    def test_idempotent_format(self):
        write(os.path.join(self.root, "changelog.d", "x.md"), "### Security\n- S\n")
        text, _ = self.run_assemble({"x.md": 4})
        self.assertIn("pull/1)).\n- S ([#4]", text)
        self.assertIn("pull/4))\n\n### Fixed", text)

    def test_main_writes_and_deletes(self):
        write(os.path.join(self.root, "changelog.d", "x.md"), "### Added\n- X\n")
        self.assertEqual(ca.main(["--root", self.root, "--version", "5.0.0-rc.1"]), 0)
        with open(os.path.join(self.root, "CHANGELOG.md")) as f:
            self.assertIn("- X", f.read())
        self.assertFalse(os.path.exists(os.path.join(self.root, "changelog.d", "x.md")))
        self.assertTrue(os.path.exists(os.path.join(self.root, "changelog.d", "README.md")))

    def test_main_dry_run_changes_nothing(self):
        write(os.path.join(self.root, "changelog.d", "x.md"), "### Added\n- X\n")
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            rc = ca.main(["--root", self.root, "--version", "5.0.0", "--dry-run"])
        self.assertEqual(rc, 0)
        with open(os.path.join(self.root, "CHANGELOG.md")) as f:
            self.assertEqual(f.read(), BASE)
        self.assertTrue(os.path.exists(os.path.join(self.root, "changelog.d", "x.md")))

    def test_check_flags_bad_fragment(self):
        write(os.path.join(self.root, "changelog.d", "bad.md"), "### Nope\n- X\n")
        self.assertEqual(ca.main(["--root", self.root, "--check"]), 1)


class GitLookupTest(unittest.TestCase):
    def git(self, *a):
        subprocess.run(["git", "-c", "user.name=t", "-c", "user.email=t@t", *a], cwd=self.root,
                       check=True, capture_output=True)

    def test_merge_commit_number(self):
        with tempfile.TemporaryDirectory() as root:
            self.root = root
            self.git("init", "-q", "-b", "master")
            write(os.path.join(root, "f"), "x")
            self.git("add", "."); self.git("commit", "-qm", "init")
            self.git("checkout", "-qb", "feat")
            write(os.path.join(root, "changelog.d", "feat.md"), "### Added\n- X\n")
            self.git("add", "."); self.git("commit", "-qm", "add fragment")
            self.git("checkout", "-q", "master")
            write(os.path.join(root, "g"), "y")
            self.git("add", "."); self.git("commit", "-qm", "other")
            self.git("merge", "--no-ff", "-qm", "Merge pull request #77 from org/feat", "feat")
            self.assertEqual(ca.git_pr_number("changelog.d/feat.md", root), 77)

    def test_squash_commit_number(self):
        with tempfile.TemporaryDirectory() as root:
            self.root = root
            self.git("init", "-q", "-b", "master")
            write(os.path.join(root, "changelog.d", "s.md"), "### Added\n- X\n")
            self.git("add", "."); self.git("commit", "-qm", "Add thing (#78)")
            self.assertEqual(ca.git_pr_number("changelog.d/s.md", root), 78)

    def test_untracked_is_none(self):
        with tempfile.TemporaryDirectory() as root:
            self.root = root
            self.git("init", "-q", "-b", "master")
            self.assertIsNone(ca.git_pr_number("changelog.d/none.md", root))


if __name__ == "__main__":
    unittest.main()
