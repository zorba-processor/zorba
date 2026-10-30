#!/usr/bin/env python3
# Copyright 2006-2016 zorba.io
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Unit tests for difftest.py (normalization, classification, selection).

Run: python3 -m unittest discover -s test/differential
"""

import decimal
import os
import re
import tempfile
import types
import unittest

import boundary
import difftest as dt
import property as prop
import xqgen


def res(out="", err="", rc=0, timeout=False):
    return {"rc": rc, "out": out, "err": err, "timeout": timeout,
            "seconds": 0.0}


NO_SPEC = {"error": [], "vars": [], "serialization": [], "unsupported": None}


class Canonical(unittest.TestCase):
    def test_xml_declaration_and_trailing_newline(self):
        self.assertEqual(dt.canonical('<?xml version="1.0" encoding="UTF-8"?>'
                                      '<a/>\n'), "<a></a>")

    def test_attribute_order_and_char_refs(self):
        self.assertEqual(dt.canonical('<a y="2" x="1">&#x41;</a>'),
                         dt.canonical("<a x='1' y='2'>A</a>"))

    def test_redundant_namespace_declarations(self):
        self.assertEqual(
            dt.canonical('<p:a xmlns:p="u"><p:b xmlns:p="u"/></p:a>'),
            dt.canonical('<p:a xmlns:p="u"><p:b/></p:a>'))

    def test_prefixes_do_not_matter(self):
        self.assertEqual(dt.canonical('<fn:a xmlns:fn="u"><fn:b/></fn:a>'),
                         dt.canonical('<a xmlns="u"><b/></a>'))
        self.assertNotEqual(dt.canonical('<a xmlns="u"/>'),
                            dt.canonical('<a xmlns="v"/>'))

    def test_atomics_unchanged(self):
        self.assertEqual(dt.canonical("1 2 3"), "1 2 3")

    def test_not_well_formed_kept(self):
        self.assertEqual(dt.canonical("a < b"), "a < b")


class Compare(unittest.TestCase):
    def test_equal(self):
        self.assertEqual(dt.compare("<a>1</a>", "<a>1</a>", ""), (True, ""))

    def test_numeric_lexical(self):
        self.assertEqual(dt.compare("1.0E0 2", "1 2", ""),
                         (True, "numeric-lexical"))

    def test_numeric_precision_is_flagged(self):
        self.assertEqual(dt.compare("0.30000000000000004", "0.3", ""),
                         (False, "numeric-precision"))

    def test_whitespace_only_is_flagged(self):
        self.assertEqual(dt.compare("<a> x</a>", "<a>x</a>", ""),
                         (False, "whitespace-only"))

    def test_unordered_only_when_query_says_so(self):
        self.assertEqual(dt.compare("1 2", "2 1", "unordered { (1, 2) }"),
                         (True, "unordered"))
        self.assertEqual(dt.compare("1 2", "2 1", "(1, 2)"), (False, ""))

    def test_group_by_without_order_by(self):
        q = "for $x in 1 to 4 group by $k := $x mod 2 return <g>{$x}</g>"
        self.assertEqual(dt.compare("<g>1 3</g><g>2 4</g>",
                                    "<g>2 4</g><g>1 3</g>", q),
                         (True, "group-order"))
        # Order within a group still matters.
        self.assertEqual(dt.compare("<g>1 3</g>", "<g>3 1</g>", q),
                         (False, ""))
        self.assertEqual(dt.compare("1 2", "2 1", q + " order by $k"),
                         (False, ""))

    def test_different(self):
        self.assertEqual(dt.compare("a", "b", ""), (False, ""))


class Classify(unittest.TestCase):
    ZERR = ('<exception xmlns="http://zorba.io/errors"><kind>dynamic</kind>'
            '<code namespace="http://www.w3.org/2005/xqt-errors" '
            'local-name="FOAR0001"/></exception>')
    SERR = ("Error at char 3 in expression in query q.xq\n"
            "  FOAR0001  Integer division by zero\n")

    def test_same_error(self):
        self.assertEqual(dt.classify(res(err=self.ZERR, rc=1),
                                     res(err=self.SERR, rc=2), ""),
                         ("agree", "both-error"))

    def test_different_error(self):
        self.assertEqual(dt.classify(res(err=self.ZERR, rc=1),
                                     res(err="XPTY0004: bad", rc=2), "")[0],
                         "different-error-code")

    def test_unknown_codes_do_not_agree(self):
        self.assertEqual(dt.classify(res(err="?", rc=1),
                                     res(err="?", rc=2), "")[0],
                         "different-error-code")

    def test_one_sided(self):
        self.assertEqual(dt.classify(res(err=self.ZERR, rc=1), res("1"), ""),
                         ("zorba-error-only", ""))
        self.assertEqual(dt.classify(res("1"), res(err=self.SERR, rc=2), ""),
                         ("saxon-error-only", ""))

    def test_crash(self):
        self.assertEqual(dt.classify(res(rc=-11), res("1"), ""),
                         ("zorba-error-only", "crash"))
        self.assertEqual(dt.zorba_error_code(res(rc=-11)), "CRASH:SIGSEGV")

    def test_timeout(self):
        self.assertEqual(dt.classify(res(timeout=True), res("1"), ""),
                         ("timeout", "zorba"))

    def test_saxon_lexical_error_code(self):
        zerr = ('<exception xmlns="http://zorba.io/errors">'
                '<code namespace="http://www.example.com" '
                'local-name="testerror"/></exception>')
        serr = ("Error on line 1 column 19 of error4.xq:\n"
                "  example:testerror  my error !\n")
        self.assertEqual(dt.saxon_error_code(res(err=serr, rc=2)),
                         "example:testerror")
        self.assertEqual(dt.classify(res(err=zerr, rc=1),
                                     res(err=serr, rc=2), ""),
                         ("agree", "both-error"))

    def test_warnings_are_not_errors(self):
        serr = ("Warning on line 12 column 40 of q.xq:\n"
                "  XPTY0004  The required item type ... is node()\n"
                "Type error on line 12 column 40 of q.xq:\n"
                "  XPTY0019  The required item type ...\n")
        self.assertEqual(dt.saxon_error_code(res(err=serr, rc=2)), "XPTY0019")
        zerr = ('<exception xmlns="http://zorba.io/warnings"><code namespace='
                '"http://zorba.io/warnings" local-name="ZWST0002"/></exception>'
                '<exception xmlns="http://zorba.io/errors"><code namespace='
                '"http://www.w3.org/2005/xqt-errors" local-name="XPTY0019"/>'
                '</exception>')
        self.assertEqual(dt.zorba_error_code(res(err=zerr, rc=1)), "XPTY0019")

    def test_saxon_java_exception(self):
        self.assertEqual(dt.saxon_error_code(res(
            err='Exception in thread "main" java.lang.StackOverflowError\n'
                '\tat net.sf.saxon.X(X.java:46)\n', rc=1)),
            "JAVA:StackOverflowError")

    def test_saxon_custom_error_code(self):
        self.assertEqual(dt.saxon_error_code(res(
            err="Error\n  Q{http://ex}my-err  boom", rc=2)),
            "Q{http://ex}my-err")
        self.assertEqual(dt.saxon_error_code(res(
            err="  Q{http://www.w3.org/2005/xqt-errors}FOER0000  x", rc=2)),
            "FOER0000")


class Selection(unittest.TestCase):
    def test_rules(self):
        cases = {
            "1 + 1": None,
            'import module namespace m = "x";\n1': "module-import",
            'declare namespace z = "http://zorba.io/modules/x"; 1':
                "zorba-specific",
            "jn:keys({})": "jsoniq",
            "#!/usr/bin/zorba -q\n1": "zorba-specific",
            '{ "a" : 1 }': "jsoniq",
            "insert node <a/> into $x": "updates",
            '$x contains text "a"': "full-text",
            "validate { <a/> }": "schema-aware",
            "current-dateTime()": "nondeterministic",
            "declare variable $x external; $x": "external-variable",
            "declare variable $x external := 1; $x": None,
            "(: insert node :) 1": None,
            "let $a := 1,\n    $b := 2 return $a": None,
            "{ $a := 2; }": "scripting",
        }
        for text, want in cases.items():
            with self.subTest(text=text):
                self.assertEqual(dt.classify_query("string/q", text, NO_SPEC),
                                 want)

    def test_excluded_directory(self):
        self.assertEqual(dt.classify_query("fulltext/q", "1", NO_SPEC),
                         "full-text")

    def test_spec(self):
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "q.spec")
            with open(p, "w") as f:
                f.write("Args:\n-x\ninput-context=$RBKT_SRC_DIR/a.xml\n"
                        "-x\nv:=abc\nError: http://www.w3.org/2005/"
                        "xqt-errors:XPTY0004\n")
            spec = dt.parse_spec(p)
        self.assertEqual(spec["vars"],
                         [("input-context", "doc", "$RBKT_SRC_DIR/a.xml"),
                          ("v", "string", "abc")])
        self.assertEqual(spec["error"],
                         ["http://www.w3.org/2005/xqt-errors:XPTY0004"])
        self.assertIsNone(spec["unsupported"])
        text = "declare variable $input-context external; $input-context"
        self.assertIsNone(dt.classify_query("w3c/q", text, spec))


class BatchFormat(unittest.TestCase):
    def test_parse_complete(self):
        self.assertEqual(xqgen.parse_batch_output(
            "#1 integer:2\n#0 ()\n#2 !FOAR0001\n", 3),
            ["()", "integer:2", "!FOAR0001"])

    def test_parse_incomplete_or_malformed(self):
        self.assertIsNone(xqgen.parse_batch_output("#0 ()\n", 2))
        self.assertIsNone(xqgen.parse_batch_output("#0 ()\nnoise\n#1 ()", 2))
        self.assertIsNone(xqgen.parse_batch_output("#0 ()\n#0 ()", 1))

    def test_xq_string(self):
        self.assertEqual(xqgen.xq_string('a"&\r\U0001F600'),
                         '"a""&amp;&#xD;&#x1F600;"')

    def test_xq_text(self):
        self.assertEqual(xqgen.xq_text('{<&>}"', attr=True),
                         "{{&lt;&amp;&gt;}}&quot;")

    def test_batch_query_numbers_cases(self):
        q = xqgen.batch_query(["1", "2"])
        self.assertIn('concat("#1 "', q)
        self.assertTrue(q.startswith('xquery version "3.0";'))


class CaseClassification(unittest.TestCase):
    def test_classes(self):
        cases = [
            (("integer:1", "integer:1"), ("agree", "")),
            (("!FOAR0001", "!FOAR0001"), ("agree", "both-error")),
            (("!FOAR0001", "!FOAR0002"), ("different-error-code", "")),
            (("!FOAR0001", "integer:1"), ("zorba-error-only", "")),
            (("!CRASH:SIGSEGV", "integer:1"), ("zorba-error-only", "crash")),
            (("integer:1", "!XPTY0004"), ("saxon-error-only", "")),
            ((None, "integer:1"), ("timeout", "zorba")),
            (("double:1.0E20", "double:1E20"), ("agree", "numeric-lexical")),
            (("double:NaN", "double:NaN"), ("agree", "")),
            (("double:0.3", "double:0.30000000000000004"),
             ("different-result", "numeric-precision")),
            (("float:2", "float:1.999"), ("different-result", "")),
            (("decimal:1", "integer:1"), ("different-result", "type-only")),
            (("string:a", "string:b"), ("different-result", "")),
            (("integer:1 integer:2", "integer:1"), ("different-result", "")),
        ]
        for (z, s), want in cases:
            with self.subTest(z=z, s=s):
                self.assertEqual(dt.classify_case(z, s), want)

    def test_property(self):
        self.assertEqual(dt.classify_property("boolean:true")[0], "pass")
        self.assertEqual(dt.classify_property("boolean:false")[0], "fail")
        self.assertEqual(dt.classify_property("()")[0], "fail")
        self.assertEqual(dt.classify_property("!XPTY0004")[0], "error")
        self.assertEqual(dt.classify_property(None)[0], "timeout")


class Batching(unittest.TestCase):
    def test_make_batches(self):
        cs = [{"id": str(i), "expr": "1", "solo": i == 2} for i in range(7)]
        sizes = [[c["id"] for c in b] for b in dt.make_batches(cs, 3)]
        self.assertEqual(sizes, [["2"], ["0", "1", "3"], ["4", "5", "6"]])

    def test_split_until_failing_case_is_alone(self):
        case_re = re.compile(r'concat\("#(\d+) ", try \{ local:show\(\(\n'
                             r'(.*?)\n\)\)', re.S)

        def fake_run(cmd, timeout, cwd):
            path = [a for a in cmd if a.endswith(".xq")][0]
            with open(path) as f:
                exprs = case_re.findall(f.read())
            if any(e == "BAD" for _, e in exprs):
                return {"rc": 1, "out": "", "timeout": False, "seconds": 0,
                        "err": "static error [err:XPTY0004]"}
            out = "\n".join("#%s integer:%s" % (n, e) for n, e in exprs)
            return {"rc": 0, "out": out, "err": "", "timeout": False,
                    "seconds": 0}

        orig = dt.run_process
        dt.run_process = fake_run
        try:
            with tempfile.TemporaryDirectory() as d:
                args = types.SimpleNamespace(zorba="zorba", rbkt=d, out=d,
                                             timeout=1)
                cs = [{"id": str(i), "expr": e, "solo": False}
                      for i, e in enumerate(["1", "2", "BAD", "4", "5"])]
                stats = dt.Stats()
                got = dt.run_side("zorba", cs, args, "t", stats)
        finally:
            dt.run_process = orig
        self.assertEqual(got, ["integer:1", "integer:2", "!XPTY0004",
                               "integer:4", "integer:5"])
        self.assertGreater(stats.splits["zorba"], 0)


class Generators(unittest.TestCase):
    def test_boundary_deterministic_and_unique(self):
        a, b = boundary.cases(), boundary.cases()
        self.assertEqual(a, b)
        self.assertEqual(len({c["id"] for c in a}), len(a))
        for c in a:
            self.assertTrue(c["expr"].isascii(), c["id"])

    def test_cast_table(self):
        self.assertTrue(boundary.cast_valid("dT", "gYear"))
        self.assertFalse(boundary.cast_valid("tim", "date"))
        self.assertTrue(boundary.cast_valid("bool", "double"))
        self.assertFalse(boundary.cast_valid("uri", "QName"))
        self.assertTrue(boundary.cast_valid("int", "string"))

    def test_property_seeded(self):
        self.assertEqual(prop.cases(5), prop.cases(5))
        self.assertNotEqual(prop.cases(5), prop.cases(6))
        cs = prop.cases(5)
        self.assertEqual(len({c["id"] for c in cs}), len(cs))
        for c in cs:
            # Element names may be non-ASCII; values use character refs.
            self.assertFalse(re.search(r"[\x00-\x08\x0a-\x1f\x7f-\x9f]", c["expr"]),
                             c["id"])

    def test_oracles(self):
        self.assertEqual(prop.substring_oracle("motor car", 6, 3), " ca")
        self.assertEqual(prop.substring_oracle("metadata", 4, 3), "ada")
        self.assertEqual(prop.substring_oracle("12345", 1.5, 2.6), "234")
        self.assertEqual(prop.substring_oracle("12345", 0, 3), "12")
        self.assertEqual(prop.subsequence_oracle([1, 2, 3, 4, 5], -0.5, 2),
                         [1])
        self.assertEqual(prop.dtd_canonical(90061), "P1DT1H1M1S")
        self.assertEqual(prop.dtd_canonical(-3600), "-PT1H")
        self.assertEqual(prop.dtd_micro(500000), "PT0.5S")
        self.assertEqual(prop.ymd_canonical(-13), "-P1Y1M")
        self.assertEqual(prop.ymd_canonical(0), "P0M")
        self.assertEqual(prop.canonical_decimal(decimal.Decimal("-0.00")),
                         "0")
        self.assertEqual(prop.canonical_decimal(decimal.Decimal("12.3400")),
                         "12.34")
        self.assertEqual(prop.encode_for_uri("a b~\u00e9"), "a%20b~%C3%A9")
        self.assertEqual(prop.float32_repr(0.1), "0.1")


if __name__ == "__main__":
    unittest.main()
