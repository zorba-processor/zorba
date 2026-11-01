"""Grading of task answers against the ground truth.

A grading rule is one of:
  {"fields": {name: rule, ...}}        an object; every listed field must pass
                                       (extra fields in the answer are ignored)
  {"type": "exact"}                    equal JSON values (1 and 1.0 are equal)
  {"type": "number", "tolerance": t}   numbers within t of each other
  {"type": "set"}                      lists equal as sets of scalars
  {"type": "set", "key": k, "items": rule}
                                       lists of objects matched by field k
                                       (same keys, no duplicates), each pair
                                       graded with rule
  {"type": "list", "items": rule}      lists of the same length, graded
                                       element by element in order
"""

import json
import numbers


def _num(x):
    return isinstance(x, numbers.Number) and not isinstance(x, bool)


def grade(answer, truth, rule, path="$"):
    """Returns None if the answer passes, else the reason it fails."""
    if "fields" in rule:
        if not isinstance(answer, dict):
            return f"{path}: expected an object, got {json.dumps(answer)[:80]}"
        for name, sub in rule["fields"].items():
            if name not in answer:
                return f"{path}.{name}: missing"
            why = grade(answer[name], truth[name], sub, f"{path}.{name}")
            if why:
                return why
        return None
    kind = rule["type"]
    if kind == "exact":
        if _num(answer) and _num(truth):
            ok = answer == truth
        else:
            ok = type(answer) is type(truth) and answer == truth
        return None if ok else (f"{path}: {json.dumps(answer)[:80]} != "
                                f"{json.dumps(truth)[:80]}")
    if kind == "number":
        if not _num(answer):
            return f"{path}: not a number: {json.dumps(answer)[:80]}"
        if abs(answer - truth) > rule["tolerance"]:
            return (f"{path}: {answer} differs from {truth} by more than "
                    f"{rule['tolerance']}")
        return None
    if kind in ("set", "list"):
        if not isinstance(answer, list):
            return f"{path}: expected a list, got {json.dumps(answer)[:80]}"
    if kind == "set" and "key" not in rule:
        canon = lambda xs: sorted(json.dumps(x, sort_keys=True) for x in xs)
        if canon(answer) != canon(truth):
            extra = sorted(set(canon(answer)) - set(canon(truth)))[:5]
            missing = sorted(set(canon(truth)) - set(canon(answer)))[:5]
            if not extra and not missing:
                return f"{path}: duplicates"
            return f"{path}: extra {extra}, missing {missing}"
        return None
    if kind == "set":
        key = rule["key"]
        if not all(isinstance(x, dict) and key in x for x in answer):
            return f"{path}: every item needs the field {key}"
        a = {json.dumps(x[key]): x for x in answer}
        t = {json.dumps(x[key]): x for x in truth}
        if len(a) != len(answer):
            return f"{path}: duplicate {key}"
        if set(a) != set(t):
            return (f"{path}: {key} values differ: extra "
                    f"{sorted(set(a) - set(t))[:5]}, missing "
                    f"{sorted(set(t) - set(a))[:5]}")
        for k in sorted(t):
            why = grade(a[k], t[k], rule["items"], f"{path}[{key}={k}]")
            if why:
                return why
        return None
    if kind == "list":
        if len(answer) != len(truth):
            return f"{path}: {len(answer)} items, expected {len(truth)}"
        for i, (x, y) in enumerate(zip(answer, truth)):
            why = grade(x, y, rule["items"], f"{path}[{i}]")
            if why:
                return why
        return None
    raise ValueError(f"unknown grading rule {rule}")
