"""Unit tests of grading.py, and of every task's grading rule against its
own ground truth (a correct answer passes, a perturbed one fails)."""

import copy
import json
import os
import unittest

import grading

HERE = os.path.dirname(os.path.abspath(__file__))


class GradingTest(unittest.TestCase):
    def test_exact(self):
        r = {"type": "exact"}
        self.assertIsNone(grading.grade("a", "a", r))
        self.assertIsNone(grading.grade(1, 1.0, r))
        self.assertIsNotNone(grading.grade("1", 1, r))
        self.assertIsNotNone(grading.grade(None, "x", r))
        self.assertIsNone(grading.grade(None, None, r))
        self.assertIsNotNone(grading.grade(True, 1, r))

    def test_number(self):
        r = {"type": "number", "tolerance": 0.1}
        self.assertIsNone(grading.grade(1.05, 1.0, r))
        self.assertIsNotNone(grading.grade(1.2, 1.0, r))
        self.assertIsNotNone(grading.grade("1.0", 1.0, r))

    def test_sets(self):
        self.assertIsNone(grading.grade(["b", "a"], ["a", "b"],
                                        {"type": "set"}))
        self.assertIsNotNone(grading.grade(["a"], ["a", "b"], {"type": "set"}))
        self.assertIsNotNone(grading.grade(["a", "a"], ["a"], {"type": "set"}))
        keyed = {"type": "set", "key": "k", "items": {"fields": {
            "k": {"type": "exact"},
            "v": {"type": "number", "tolerance": 0.5}}}}
        truth = [{"k": "x", "v": 1}, {"k": "y", "v": 2}]
        self.assertIsNone(grading.grade(
            [{"k": "y", "v": 2.2}, {"k": "x", "v": 1, "extra": 0}], truth,
            keyed))
        self.assertIsNotNone(grading.grade([{"k": "x", "v": 1}], truth, keyed))
        self.assertIsNotNone(grading.grade(
            [{"k": "x", "v": 1}, {"k": "y", "v": 3}], truth, keyed))

    def test_list_and_fields(self):
        r = {"fields": {"l": {"type": "list", "items": {"type": "exact"}}}}
        self.assertIsNone(grading.grade({"l": [1, 2]}, {"l": [1, 2]}, r))
        self.assertIsNotNone(grading.grade({"l": [2, 1]}, {"l": [1, 2]}, r))
        self.assertIsNotNone(grading.grade({}, {"l": [1, 2]}, r))
        self.assertIsNotNone(grading.grade([1, 2], {"l": [1, 2]}, r))


def perturb(value):
    """A clearly wrong variant of a ground-truth value, or None."""
    if isinstance(value, bool) or value is None:
        return "wrong"
    if isinstance(value, (int, float)):
        return value + 1000
    if isinstance(value, str):
        return value + "-wrong"
    if isinstance(value, list):
        return value[:-1] if value else ["wrong"]
    return None


class TaskRulesTest(unittest.TestCase):
    def test_every_task(self):
        with open(os.path.join(HERE, "tasks", "tasks.json")) as f:
            tasks = json.load(f)["tasks"]
        with open(os.path.join(HERE, "tasks", "ground_truth.json")) as f:
            truth = json.load(f)
        self.assertEqual(sorted(t["id"] for t in tasks), sorted(truth))
        for t in tasks:
            gt = truth[t["id"]]
            self.assertIsNone(grading.grade(gt, gt, t["grading"]), t["id"])
            self.assertEqual(sorted(t["answer_schema"]["required"]),
                             sorted(t["grading"]["fields"]), t["id"])
            for field in t["grading"]["fields"]:
                bad = copy.deepcopy(gt)
                bad[field] = perturb(gt[field])
                self.assertIsNotNone(
                    grading.grade(bad, gt, t["grading"]),
                    f"{t['id']}: perturbed {field} still passes")


if __name__ == "__main__":
    unittest.main()
