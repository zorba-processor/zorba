import os
import subprocess
import sys
import tempfile
import unittest

SCRIPT = os.path.join(os.path.dirname(__file__), "..",
                      "summarize_sanitizer_logs.py")

BASELINE = """# header
known/one | Failed | backlog 1
known/with space | Subprocess aborted | backlog 2
known/flaky | flaky | backlog 3
"""


def ctest_log(failed, total=10, listing=None, summary=True):
    """failed: list of (list entry text); listing overrides the entries."""
    lines = ["   1/10 Test    #1: known/one ........   Passed    0.01 sec"]
    if summary:
        lines += ["", f"{100 - 10 * len(failed)}% tests passed, "
                      f"{len(failed)} tests failed out of {total}", "",
                  "Total Test time (real) = 1.00 sec", ""]
    if listing is None:
        listing = failed
    if listing:
        lines.append("The following tests FAILED:")
        lines += [f"\t{i + 3} - {e}" for i, e in enumerate(listing)]
        lines.append("Errors while running CTest")
    return "\n".join(lines) + "\n"


class CompareTest(unittest.TestCase):
    def run_compare(self, log, rc=None):
        with tempfile.TemporaryDirectory() as d:
            paths = {}
            for name, text in (("baseline", BASELINE), ("log", log),
                               ("rc", None if rc is None else f"{rc}\n")):
                if text is not None:
                    paths[name] = os.path.join(d, name)
                    with open(paths[name], "w") as f:
                        f.write(text)
            cmd = [sys.executable, "-I", SCRIPT, "--baseline",
                   paths["baseline"], "--ctest-log", paths["log"]]
            if rc is not None:
                cmd += ["--ctest-exit-code", paths["rc"]]
            p = subprocess.run(cmd, capture_output=True, text=True)
            return p.returncode, p.stdout

    def test_known_failures_pass(self):
        code, _ = self.run_compare(
            ctest_log(["known/one (Failed)",
                       "known/with space (Subprocess aborted)"]), rc=8)
        self.assertEqual(code, 0)

    def test_new_failure_fails(self):
        code, out = self.run_compare(ctest_log(["other/new (Failed)"]), rc=8)
        self.assertEqual(code, 1)
        self.assertIn("other/new", out)

    def test_changed_kind_fails(self):
        code, _ = self.run_compare(ctest_log(["known/one (Timeout)"]), rc=8)
        self.assertEqual(code, 1)

    def test_flaky_may_fail_with_any_kind(self):
        code, _ = self.run_compare(ctest_log(["known/flaky (Timeout)"]),
                                   rc=8)
        self.assertEqual(code, 0)

    def test_new_failure_with_spaced_name_is_reported(self):
        code, out = self.run_compare(
            ctest_log(["brand new test (Failed)"]))
        self.assertEqual(code, 1)
        self.assertIn("brand new test", out)

    def test_labels_after_the_kind_are_ignored(self):
        code, out = self.run_compare(
            ctest_log(["other/labelled (Failed)        L1 L2"]))
        self.assertEqual(code, 1)
        self.assertIn("other/labelled", out)

    def test_count_mismatch_fails(self):
        # The summary says 2 failed but the list has one entry that parses.
        log = ctest_log(["known/one (Failed)", "known/flaky (Failed)"],
                        listing=["known/one (Failed)"])
        code, out = self.run_compare(log)
        self.assertEqual(code, 1)
        self.assertIn("reports 2 failed", out)

    def test_unparsable_entry_fails(self):
        code, out = self.run_compare(ctest_log(["malformed entry"]))
        self.assertEqual(code, 1)
        self.assertIn("could not be parsed", out)

    def test_truncated_log_fails(self):
        code, out = self.run_compare(ctest_log([], summary=False))
        self.assertEqual(code, 1)
        self.assertIn("no ctest summary", out)

    def test_summary_text_inside_test_output_is_not_a_summary(self):
        log = ctest_log([], summary=False) + \
            "output: 100% tests passed, 0 tests failed out of 5 (not ctest)\n"
        code, _ = self.run_compare(log)
        self.assertEqual(code, 1)

    def test_rc_8_without_listed_failures_fails(self):
        code, out = self.run_compare(ctest_log([]), rc=8)
        self.assertEqual(code, 1)
        self.assertIn("exited 8", out)

    def test_rc_other_than_0_or_8_fails(self):
        code, _ = self.run_compare(ctest_log([]), rc=1)
        self.assertEqual(code, 1)

    def test_missing_exit_code_fails(self):
        with tempfile.TemporaryDirectory() as d:
            rc_path = os.path.join(d, "absent")
            base, log = (os.path.join(d, n) for n in ("b", "l"))
            open(base, "w").write(BASELINE)
            open(log, "w").write(ctest_log([]))
            p = subprocess.run(
                [sys.executable, "-I", SCRIPT, "--baseline", base,
                 "--ctest-log", log, "--ctest-exit-code", rc_path],
                capture_output=True, text=True)
            self.assertEqual(p.returncode, 1)

    def test_rc_0_all_passed(self):
        code, _ = self.run_compare(ctest_log([]), rc=0)
        self.assertEqual(code, 0)


if __name__ == "__main__":
    unittest.main()
