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

import os
import tempfile
import unittest

import difftest as dt


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
        zerr = ('<code namespace="http://www.example.com" '
                'local-name="testerror"/>')
        serr = ("Error on line 1 column 19 of error4.xq:\n"
                "  example:testerror  my error !\n")
        self.assertEqual(dt.saxon_error_code(res(err=serr, rc=2)),
                         "example:testerror")
        self.assertEqual(dt.classify(res(err=zerr, rc=1),
                                     res(err=serr, rc=2), ""),
                         ("agree", "both-error"))

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


if __name__ == "__main__":
    unittest.main()
