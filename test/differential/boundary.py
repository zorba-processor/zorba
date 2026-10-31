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

"""Boundary-value matrices: deterministic XQuery 3.0 cases that combine
boundary values of each atomic type with operators, casts and the main
F&O functions. The set is the same on every run (no randomness), so
results can be compared across runs.

Families (first id segment): arith, unary, typeerr, valcmp, gencmp, cast,
castable, lexcast, string, string2, numfn, date, datearith, duration,
aggregate, qname, boolean.
"""

import itertools

from xqgen import number_cases, xq_string


def ua(s):
    return "xs:untypedAtomic(%s)" % xq_string(s)


def ctor(t, s):
    return "xs:%s(%s)" % (t, xq_string(s))


# (type, expression). Types: int dec dbl flt str ua bool dur ymd dtd dT dat
# tim gYM gY gMD gD gM hex b64 uri qn.
INT = [("int", e) for e in [
    "0", "xs:integer('-0')", "1", "-1", "7", "-7", "2147483648",
    "4294967296", "9223372036854775806", "9223372036854775807",
    "-9223372036854775807", "xs:integer('-9223372036854775808')"]]
DEC = [("dec", e) for e in [
    "0.0", "-0.0", "0.5", "-0.5", "2.5", "-2.5",
    "1.0000000000000000000001", "123456789012345678.123456789",
    "0.000000000000000000000000000001", "-999999999999999999.9999999999"]]
DBL = [("dbl", e) for e in [
    "0e0", "-0e0", "xs:double('INF')", "xs:double('-INF')",
    "xs:double('NaN')", "1.7976931348623157e308", "4.9e-324",
    "2.2250738585072014e-308", "2.225073858507201e-308", "0.1e0", "1e16",
    "9007199254740993e0", "1.0000000000000002e0", "-2.5e0", "0.5e0"]]
FLT = [("flt", ctor("float", s)) for s in [
    "0", "-0", "INF", "-INF", "NaN", "3.4028235E38", "1.4E-45",
    "1.17549435E-38", "0.1", "16777217", "-2.5"]]
NUM = INT + DEC + DBL + FLT

# A few values per numeric type for the cross-type matrices.
CORE = ([INT[i] for i in (0, 2, 3, 9, 11)] +
        [DEC[i] for i in (1, 4, 5, 6)] +
        [DBL[i] for i in (1, 2, 4, 5, 6, 9)] +
        [FLT[i] for i in (1, 3, 4, 5, 8)])

UA = [("ua", ua(s)) for s in ["1", "-0", " 2 ", "INF", "1e400", "abc", ""]]

STRINGS = [
    "", "a", "abc", "ABC xyz", "  a \t b\n  ", "é", "é",
    "\U0001F600", "a\U0001F600b", "퟿�", "‮abc‬",
    "אבc", "ß", "ǅ", "İ", "ﬃ", "Σ",
    "ΑΣ", "x" * 1000 + "\U00010348", "123", " 1e3 ", "-0", "INF",
    "true", "\t\n\r", "<&>\"'", "a,b,,c", "\u0085", "\u007f", " ",
    " ", "Ǻ",
]
STR = [("str", xq_string(s)) for s in STRINGS]
# Smaller set for the pairwise matrices.
STR2 = [STR[i] for i in (0, 1, 2, 5, 6, 7, 8, 12, 13, 16, 19, 25, 26, 31)]

BOOL = [("bool", "true()"), ("bool", "false()")]

DATES = ["2000-02-29", "1900-02-28", "2024-02-29Z", "9999-12-31",
         "-9999-01-01", "0001-01-01", "-0001-12-31", "2000-01-01+14:00",
         "2000-01-01-14:00", "1970-01-01Z", "2001-12-31-00:00"]
DATETIMES = ["2000-02-29T12:00:00", "1999-12-31T23:59:59.999999999Z",
             "2000-01-01T24:00:00", "9999-12-31T23:59:59Z",
             "-9999-01-01T00:00:00Z", "2000-03-01T00:00:00+14:00",
             "2000-02-28T23:00:00-14:00", "1972-12-31T23:59:59.5-05:30",
             "0001-01-01T00:00:00.000001Z"]
TIMES = ["00:00:00", "23:59:59.999999", "24:00:00", "12:00:00+14:00",
         "12:00:00-14:00", "00:00:00Z", "13:20:00.5-05:00"]
DAT = [("dat", ctor("date", s)) for s in DATES]
DT = [("dT", ctor("dateTime", s)) for s in DATETIMES]
TIM = [("tim", ctor("time", s)) for s in TIMES]

DUR = ([("dur", ctor("duration", s)) for s in
        ["P0D", "-P1Y2M3DT4H5M6.789S", "P1Y1D", "PT1.5S"]] +
       [("ymd", ctor("yearMonthDuration", s)) for s in
        ["P1Y", "-P11M", "P0M", "P9999Y", "P768614336404564650Y"]] +
       [("dtd", ctor("dayTimeDuration", s)) for s in
        ["PT0S", "P1DT0.000001S", "-PT1S", "P10675199DT2H48M5.4775807S",
         "PT24H", "P365D", "-PT0.000000001S"]])

GTYPES = ([("gY", ctor("gYear", s)) for s in ["-0001", "2000Z", "9999"]] +
          [("gYM", ctor("gYearMonth", s)) for s in ["2000-02", "-0001-12"]] +
          [("gMD", ctor("gMonthDay", s)) for s in ["--02-29", "--12-31Z"]] +
          [("gD", ctor("gDay", s)) for s in ["---31", "---01+14:00"]] +
          [("gM", ctor("gMonth", s)) for s in ["--12", "--01-14:00"]])

BIN = ([("hex", ctor("hexBinary", s)) for s in ["", "FF00", "0a"]] +
       [("b64", ctor("base64Binary", s)) for s in ["", "AAAA", "/w=="]])

URI = [("uri", ctor("anyURI", s)) for s in
       ["", "http://a/b c", "%", "urn:x", "../a", "http://例.jp",
        "http://[::1]/"]]

QN = [("qn", e) for e in [
    "fn:QName('', 'a')", "fn:QName('http://e', 'p:a')",
    "fn:QName('http://e', 'a')", "xs:QName('xs:integer')"]]

ALL = (NUM + UA + STR + BOOL + DAT + DT + TIM + DUR + GTYPES + BIN + URI +
       QN)

NUMERIC = {"int", "dec", "dbl", "flt"}
ARITH_OPS = ["+", "-", "*", "div", "idiv", "mod"]
CMP_OPS = ["eq", "ne", "lt", "le", "gt", "ge"]

# One representative per type for the type-error matrices.
REPS = {}
for _t, _e in ALL:
    REPS.setdefault(_t, _e)


def p(e):
    return "(%s)" % e


# --------------------------------------------------------------------------
# Static typing rules (XQuery 3.0 / F&O 3.0), used to mark cases that must
# raise a type error as solo: processors may report those statically, which
# would abort the whole batch.
# --------------------------------------------------------------------------

def _num(t):
    return t in NUMERIC or t == "ua"


def arith_valid(op, a, b):
    if _num(a) and _num(b):
        return True
    dur = {"ymd", "dtd"}
    if op == "-" and a == b and a in ("dat", "dT", "tim"):
        return True
    if op in ("+", "-") and a in ("dat", "dT") and b in dur:
        return True
    if op == "+" and a in dur and b in ("dat", "dT"):
        return True
    if op in ("+", "-") and a == "tim" and b == "dtd":
        return True
    if op == "+" and a == "dtd" and b == "tim":
        return True
    if op in ("+", "-", "div") and a == b and a in dur:
        return True
    if op in ("*", "div") and a in dur and _num(b):
        return True
    if op == "*" and _num(a) and b in dur:
        return True
    return False


CMP_GROUP = {"int": "num", "dec": "num", "dbl": "num", "flt": "num",
             "str": "str", "ua": "str", "uri": "str", "bool": "bool",
             "dur": "dur", "ymd": "dur", "dtd": "dur", "hex": "hex",
             "b64": "b64", "qn": "qn"}


def cmp_valid(op, a, b):
    ga, gb = CMP_GROUP.get(a, a), CMP_GROUP.get(b, b)
    if ga != gb:
        return False
    if op in ("eq", "ne"):
        return True
    if ga == "dur":
        return a == b and a in ("ymd", "dtd")
    return ga not in ("qn", "gY", "gYM", "gMD", "gD", "gM")


CAST_TARGETS = ["untypedAtomic", "string", "boolean", "decimal", "integer",
                "float", "double", "duration", "yearMonthDuration",
                "dayTimeDuration", "dateTime", "time", "date", "gYearMonth",
                "gYear", "gMonthDay", "gDay", "gMonth", "hexBinary",
                "base64Binary", "anyURI", "QName"]
DERIVED_INT = ["long", "int", "short", "byte", "unsignedLong", "unsignedInt",
               "unsignedByte", "nonNegativeInteger", "positiveInteger",
               "negativeInteger", "nonPositiveInteger"]
DERIVED_STR = ["normalizedString", "token", "language", "NMTOKEN", "Name",
               "NCName"]
TARGET_TYPE = {"untypedAtomic": "ua", "string": "str", "boolean": "bool",
               "decimal": "dec", "integer": "int", "float": "flt",
               "double": "dbl", "duration": "dur",
               "yearMonthDuration": "ymd", "dayTimeDuration": "dtd",
               "dateTime": "dT", "time": "tim", "date": "dat",
               "gYearMonth": "gYM", "gYear": "gY", "gMonthDay": "gMD",
               "gDay": "gD", "gMonth": "gM", "hexBinary": "hex",
               "base64Binary": "b64", "anyURI": "uri", "QName": "qn"}
for _t in DERIVED_INT:
    TARGET_TYPE[_t] = "int"
for _t in DERIVED_STR:
    TARGET_TYPE[_t] = "str"


def cast_valid(src, target):
    """Casting table of XQuery 3.0 section 3.12 / F&O 3.0 section 19."""
    t = TARGET_TYPE[target]
    if t == "qn":
        return src in ("str", "qn")
    if src in ("str", "ua") or t in ("str", "ua"):
        return True
    num_bool = NUMERIC | {"bool"}
    if src in num_bool and t in num_bool:
        return True
    durs = {"dur", "ymd", "dtd"}
    if src in durs and t in durs:
        return True
    gs = {"gYM", "gY", "gMD", "gD", "gM"}
    if src == "dT" and t in gs | {"dT", "tim", "dat"}:
        return True
    if src == "dat" and t in gs | {"dT", "dat"}:
        return True
    if src in ("hex", "b64") and t in ("hex", "b64"):
        return True
    return src == t


# --------------------------------------------------------------------------
# Families
# --------------------------------------------------------------------------

def arith():
    pairs = []
    for pool in (INT, DEC, DBL, FLT):
        pairs += [(a, b, pool[0][0]) for a in pool for b in pool]
    for a in CORE:
        for b in CORE:
            if a[0] != b[0]:
                pairs.append((a, b, "mixed"))
    for u in UA:
        for c in CORE[::2]:
            pairs.append((u, c, "untyped"))
            pairs.append((c, u, "untyped"))
    for (ta, ea), (tb, eb), sub in pairs:
        for op in ARITH_OPS:
            yield ("arith", sub, "%s %s %s" % (p(ea), op, p(eb)), False)
    # Literal forms whose parse already is a boundary.
    for e in ["-9223372036854775808", "9223372036854775808",
              "-9223372036854775807 - 1", "9223372036854775807 + 1",
              "-9223372036854775807 - 2", "9223372036854775807 * -1",
              "xs:integer('-9223372036854775808') div -1",
              "xs:integer('-9223372036854775808') idiv -1",
              "xs:integer('-9223372036854775808') mod -1",
              "xs:integer('-9223372036854775808') * -1",
              "1 div 0", "1 idiv 0", "1 mod 0", "1.0 div 0", "1.0 idiv 0.0",
              "1e0 idiv 0", "xs:double('INF') idiv 1", "xs:double('NaN') idiv 1",
              "1e308 idiv 1e-308", "10 idiv 3", "-10 idiv 3", "10 idiv -3",
              "-10 mod 3", "10 mod -3", "-10.5 mod 3", "5.5 mod -2",
              "-0e0 mod 1", "1 mod -0e0", "4.5e0 mod 1.2e0",
              "1 div 3", "2 div 3", "-1 div 3", "1.0 div 3.0",
              "0.1 + 0.2", "0.1e0 + 0.2e0", "xs:float(0.1) + xs:float(0.2)",
              "1e308 * 10", "-1e308 * 10", "4.9e-324 div 2",
              "xs:float('3.4028235E38') * 2", "1 div 1e-320",
              "99999999999999999999999999999.9 + 0.1",
              "1.0000000000000000000000000001 * 1.0000000000000000000000000001",
              "123456789012345678901234567890 div 7"]:
        yield ("arith", "literal", e, False)


def unary():
    for t, e in NUM + UA:
        yield ("unary", "minus", "-%s" % p(e), False)
        yield ("unary", "plus", "+%s" % p(e), False)
        yield ("unary", "double-minus", "-(-%s)" % p(e), False)


def typeerr():
    types = ["str", "bool", "dat", "dT", "tim", "ymd", "dtd", "dur", "uri",
             "qn", "hex", "int", "dbl", "ua", "gY"]
    for a in types:
        for b in types:
            for op in ("+", "-", "*", "div", "idiv", "mod"):
                if arith_valid(op, a, b):
                    continue
                # All operators for a few pairs, + and * for the rest.
                if op not in ("+", "*") and (a, b) not in (
                        ("str", "int"), ("int", "str"), ("bool", "int"),
                        ("dat", "dat"), ("ymd", "dtd")):
                    continue
                yield ("typeerr", "arith",
                       "%s %s %s" % (p(REPS[a]), op, p(REPS[b])), True)
            for op in ("eq", "lt"):
                if op == "lt" and a not in ("int", "str"):
                    continue
                if not cmp_valid(op, a, b):
                    yield ("typeerr", "valcmp",
                           "%s %s %s" % (p(REPS[a]), op, p(REPS[b])), True)
    for t in sorted(REPS):
        for op in ("-", "+"):
            if t not in NUMERIC and t != "ua":
                yield ("typeerr", "unary", "%s%s" % (op, p(REPS[t])), True)


def valcmp():
    pairs = []
    for pool in (INT, DEC, DBL, FLT):
        pairs += [(a, b, pool[0][0]) for a in pool for b in pool]
    for a in CORE:
        for b in CORE:
            if a[0] != b[0]:
                pairs.append((a, b, "mixed"))
    for a in STR2:
        for b in STR2:
            pairs.append((a, b, "str"))
    for pool, sub in ((DAT, "date"), (DT, "dateTime"), (TIM, "time"),
                      (DUR, "duration"), (GTYPES, "g"), (BIN, "binary"),
                      (URI + STR2[:4], "uri"), (QN, "qname"), (BOOL, "bool")):
        for a in pool:
            for b in pool:
                pairs.append((a, b, sub))
    for (ta, ea), (tb, eb), sub in pairs:
        ops = (CMP_OPS if sub in NUMERIC or sub in ("mixed", "str")
               else ["eq", "lt"])
        for op in ops:
            yield ("valcmp", sub, "%s %s %s" % (p(ea), op, p(eb)),
                   not cmp_valid(op, ta, tb))


def gencmp():
    lefts = UA + [("seq", "()"), ("seq", "(1, 2)"), ("seq", "(1, 'a')"),
                  ("seq", "('a', 1)"), ("seq", "(xs:double('NaN'), 1)")]
    rights = ([CORE[i] for i in (0, 1, 5, 9, 11)] +
              [STR[i] for i in (0, 1, 19, 20, 22)] +
              [BOOL[0], DAT[0], DUR[5], URI[3], ("seq", "()"),
               ("seq", "(1, 2)")])
    for (ta, ea) in lefts:
        for (tb, eb) in rights:
            for op in ("=", "!=", "<", ">="):
                # untypedAtomic and string compare as strings; the static
                # type of the other operand decides whether it is an error.
                solo = (ta == "seq" and "'a'" in ea and tb in NUMERIC) or \
                    (ta == "seq" and tb in ("str", "dat", "ymd", "uri", "bool")
                     and ea not in ("()",))
                yield ("gencmp", "untyped" if ta == "ua" else "seq",
                       "%s %s %s" % (p(ea), op, p(eb)), solo)


def casts():
    seen_invalid = set()
    for t, e in ALL:
        for target in CAST_TARGETS:
            if t == "str" and len(e) > 100:
                continue
            valid = cast_valid(t, target)
            if target == "QName" and t != "qn":
                # Casting a non-literal to xs:QName is a type error
                # (XPTY0117 for untypedAtomic): keep one case per type.
                valid = False
            if not valid:
                if (t, target) in seen_invalid:
                    continue
                seen_invalid.add((t, target))
            yield ("cast", t, "%s cast as xs:%s" % (p(e), target), not valid)
            yield ("castable", t, "%s castable as xs:%s" % (p(e), target),
                   False)
        if t in NUMERIC or t in ("ua", "bool"):
            for target in DERIVED_INT:
                yield ("cast", t + "-derived",
                       "%s cast as xs:%s" % (p(e), target), False)
        if t == "str" and len(e) < 100:
            for target in DERIVED_STR:
                yield ("cast", "str-derived",
                       "%s cast as xs:%s" % (p(e), target), False)
    for t, e in NUM + DUR + DAT + DT + TIM:
        yield ("cast", "string-roundtrip",
               "string(%s) cast as xs:%s eq %s" % (
                   p(e), {"int": "integer", "dec": "decimal",
                          "dbl": "double", "flt": "float", "dur": "duration",
                          "ymd": "yearMonthDuration",
                          "dtd": "dayTimeDuration", "dat": "date",
                          "dT": "dateTime", "tim": "time"}[t], p(e)), False)


LEXICAL = [
    "0", "-0", "+0", "1", "+1", " 1 ", "1.", ".5", "-.5", "1.5e3", "1E",
    "e3", "1e+3", "1e-400", "1e400", "INF", "+INF", "-INF", "inf", "NaN",
    "nan", "0x1A", "1_000", "١", "9223372036854775808",
    "-9223372036854775809", "00000000000000000000000001",
    "0.00000000000000000000000000000000000001", "1.0e0", "-0.0e-0",
    "true", "false", "TRUE", " true ",
    "2000-02-29", "2001-02-29", "2000-02-30", "0000-01-01", "-0000-01-01",
    "-0001-01-01", "10000-01-01", "2000-1-01", "2000-01-01Z",
    "2000-01-01+14:00", "2000-01-01+14:01", "2000-01-01-00:00",
    "2000-01-01+15:00", "2000-01-01T00:00:00", "2000-01-01T24:00:00",
    "2000-01-01T24:00:01", "2000-01-01T23:59:60",
    "2000-01-01T12:00:00.123456789012Z", "2000-01-01T12:00", "12:00:00",
    "24:00:00", "24:00:00.1", "12:00:00.", "12:60:00", "\t12:00:00\n",
    "P", "PT", "P1Y", "P1Y-1M", "-P0D", "P1.5Y", "PT1.S", "PT.5S",
    "P0Y0M0DT0H0M0.0S", "P1M1D", "-PT0.000000001S", "P99999999999999999999Y",
    "PT9223372036854775808S", "P1DT", "PT60M", "P13M",
    "--02-29", "--02-30", "---31", "---32", "--13", "2000-13", "-0001",
    "", "F", "FF", "ff", "0G", "AAAA", "AAA=", "A===", "AA==", "AA=A",
    " AAAA ", "AA AA",
    "http://a/b c", "%", "::", "é", "a", "p:a", "xs:integer", ":a",
    "a:", "1a",
]


def lexcast():
    for s in LEXICAL:
        lit = xq_string(s)
        for target in CAST_TARGETS + ["long", "unsignedLong", "byte",
                                      "language", "NCName"]:
            if target == "QName":
                yield ("lexcast", "QName", "%s cast as xs:QName" % lit, True)
                continue
            yield ("lexcast", target, "%s cast as xs:%s" % (lit, target),
                   False)
            yield ("lexcast", target + "-castable",
                   "%s castable as xs:%s" % (lit, target), False)
            yield ("lexcast", target + "-untyped",
                   "%s cast as xs:%s" % (ua(s), target), False)


STRING_FNS = [
    "string-length(%s)", "upper-case(%s)", "lower-case(%s)",
    "normalize-space(%s)", "normalize-unicode(%s)",
    "normalize-unicode(%s, 'NFD')", "normalize-unicode(%s, 'NFKC')",
    "normalize-unicode(%s, 'NFKD')", "normalize-unicode(%s, '')",
    "normalize-unicode(%s, 'nfc')", "string-to-codepoints(%s)",
    "codepoints-to-string(reverse(string-to-codepoints(%s)))",
    "encode-for-uri(%s)", "iri-to-uri(%s)", "escape-html-uri(%s)",
    "substring(%s, 2)", "substring(%s, 0, 3)", "substring(%s, 2, 1)",
    "substring(%s, 1.5, 2.6)", "substring(%s, 0, xs:double('INF'))",
    "substring(%s, xs:double('-INF'), xs:double('INF'))",
    "substring(%s, 1, xs:double('NaN'))", "substring(%s, -1, 3)",
    "tokenize(%s, ' ')", "tokenize(%s, '\\s+')", "tokenize(%s, ',')",
    "translate(%s, 'abc', 'AB')", "replace(%s, '.', 'x')",
    "replace(%s, '^', '-')", "replace(%s, '(\\p{L})', '[$1]')",
    "matches(%s, '^.$')", "matches(%s, '\\p{Lu}')", "matches(%s, '^\\w+$')",
    "matches(%s, '^\\s*$')", "matches(%s, '.', 's')", "matches(%s, 'A', 'i')",
    "boolean(%s)", "number(%s)", "compare(%s, '')",
    "substring-before(%s, 'b')", "substring-after(%s, 'b')",
    "contains(%s, '')", "concat(%s, %s)", "string-join((%s, %s), '|')",
    "lower-case(upper-case(%s))", "string-length(upper-case(%s))",
    "starts-with(%s, '')",
]


def string_fns():
    for t, e in STR:
        for f in STRING_FNS:
            yield ("string", f.split("(", 1)[0],
                   f % tuple([e] * f.count("%s")), False)
    for t, e in UA[:3]:
        for f in STRING_FNS[:4]:
            yield ("string", "untyped", f % e, False)
    for e in ["codepoints-to-string(0)", "codepoints-to-string(1)",
              "codepoints-to-string(9)", "codepoints-to-string(55296)",
              "codepoints-to-string(57343)", "codepoints-to-string(65534)",
              "codepoints-to-string(65535)", "codepoints-to-string(1114111)",
              "codepoints-to-string(1114112)", "codepoints-to-string(-1)",
              "codepoints-to-string(())", "string-to-codepoints(())",
              "string-length(())", "upper-case(())", "substring((), 1)",
              "concat('a', (), 'b')", "string-join((), ',')",
              "tokenize('', ',')", "tokenize('abc', '')",
              "tokenize('a,b', ',*')", "replace('abc', '', 'x')",
              "replace('abc', 'b', '$')", "replace('abc', 'b', '\\')",
              "replace('abc', '(b)', '$2')", "replace('abc', 'b', '$0$0')",
              "matches('a', '[')", "matches('a', 'a', 'q')",
              "matches('a b', 'a b', 'x')", "matches('\U0001F600', '^.$')",
              "matches('\U0001F600', '^[\U0001F600]$')",
              "translate('abc', '', 'x')", "translate('aaa', 'aa', 'bc')",
              "normalize-unicode('a', 'FOO')", "normalize-unicode('a', ' NFC ')",
              "encode-for-uri('~!*()''')", "iri-to-uri(' <>\"{}|\\^`')",
              "compare('a', 'a', 'http://www.w3.org/2005/xpath-functions/"
              "collation/codepoint')",
              "compare('a', 'a', 'http://example.org/no-such-collation')",
              "string-join(('a', 'b'))", "string-length('\U0001F600\U0001F600')",
              "substring('\U0001F600ab', 2, 1)",
              "upper-case('ß')", "lower-case('İ')",
              "upper-case('ŉ')", "lower-case('ΣΣ')",
              "string-length(normalize-unicode('ﬃ', 'NFKC'))",
              "string(<a>&#xD;</a>)", "string(<a>&#x85;</a>)",
              "string-to-codepoints('&#xD;&#xA;')"]:
        lit = e.replace("\U0001F600", "&#x1F600;").replace(
            "ß", "&#xDF;").replace("İ", "&#x130;").replace(
            "ŉ", "&#x149;").replace("Σ", "&#x3A3;").replace(
            "ﬃ", "&#xFB03;")
        yield ("string", "edge", lit, False)


def string_pairs():
    fns = ["compare(%s, %s)", "contains(%s, %s)", "starts-with(%s, %s)",
           "ends-with(%s, %s)", "substring-before(%s, %s)",
           "substring-after(%s, %s)", "codepoint-equal(%s, %s)",
           "concat(%s, %s)", "string-length(concat(%s, %s))"]
    for (ta, a), (tb, b) in itertools.product(STR2, STR2):
        for f in fns:
            yield ("string2", f.split("(", 1)[0], f % (a, b), False)


NUM_FNS = [
    "abs(%s)", "ceiling(%s)", "floor(%s)", "round(%s)",
    "round(%s, 2)", "round(%s, -1)", "round(%s, -20)", "round(%s, 20)",
    "round-half-to-even(%s)", "round-half-to-even(%s, 1)",
    "round-half-to-even(%s, -1)", "round-half-to-even(%s, 400)",
    "string(%s)", "number(%s)", "xs:string(%s)", "-%s * 1",
    "format-number(%s, '#,##0.00')", "format-number(%s, '0.###E0')",
    "format-number(%s, '#')", "format-number(%s, '0.0##########################')",
    "format-number(%s, '#%%')", "%s instance of xs:integer",
]


def numfn():
    for t, e in NUM + UA[:4]:
        for f in NUM_FNS:
            yield ("numfn", f.split("(", 1)[0].strip("-% ") or "expr",
                   _fmt(f, p(e)), False)
    for t, e in INT:
        for pic in ("1", "001", "w", "W", "Ww", "i", "I", "a", "A",
                    "#,##0", "1;o"):
            yield ("numfn", "format-integer",
                   "format-integer(%s, %s)" % (p(e), xq_string(pic)), False)


def _fmt(f, arg):
    # Substitute every %s with arg, keeping a literal %% as %.
    return f.replace("%%", "\x00").replace("%s", arg).replace("\x00", "%")


DATE_FNS = {
    "dat": ["year-from-date(%s)", "month-from-date(%s)", "day-from-date(%s)",
            "timezone-from-date(%s)", "string(%s)", "xs:dateTime(%s)",
            "format-date(%s, '[Y0001]-[M01]-[D01]')",
            "format-date(%s, '[F1] [d] [W] [w]')", "format-date(%s, '[Y]')",
            "format-date(%s, '[Y,2]')", "format-date(%s, '[D1o] [MI] [YI]')",
            "format-date(%s, '[Z]')"],
    "dT": ["year-from-dateTime(%s)", "month-from-dateTime(%s)",
           "day-from-dateTime(%s)", "hours-from-dateTime(%s)",
           "minutes-from-dateTime(%s)", "seconds-from-dateTime(%s)",
           "timezone-from-dateTime(%s)", "string(%s)", "xs:date(%s)",
           "xs:time(%s)", "xs:gYear(%s)", "xs:gMonthDay(%s)",
           "format-dateTime(%s, '[Y0001]-[M01]-[D01]T[H01]:[m01]:[s01].[f001][Z]')",
           "format-dateTime(%s, '[f1] [f000001] [f9]')",
           "format-dateTime(%s, '[H] [h] [m] [s]')",
           "dateTime(xs:date(%s), xs:time(%s))"],
    "tim": ["hours-from-time(%s)", "minutes-from-time(%s)",
            "seconds-from-time(%s)", "timezone-from-time(%s)", "string(%s)",
            "format-time(%s, '[H01]:[m01]:[s01].[f001] [Z0]')",
            "format-time(%s, '[h].[m01] [P]')"],
}
TZS = ["()", "xs:dayTimeDuration('PT0S')", "xs:dayTimeDuration('PT14H')",
       "xs:dayTimeDuration('-PT14H')", "xs:dayTimeDuration('PT10H30M')",
       "xs:dayTimeDuration('-PT14H1M')", "xs:dayTimeDuration('PT1S')",
       "xs:dayTimeDuration('P1D')"]
ADJ = {"dat": "adjust-date-to-timezone", "dT": "adjust-dateTime-to-timezone",
       "tim": "adjust-time-to-timezone"}


def date_fns():
    for t, e in DAT + DT + TIM:
        for f in DATE_FNS[t]:
            yield ("date", f.split("(", 1)[0], _fmt(f, p(e)), False)
        for tz in TZS:
            yield ("date", ADJ[t], "%s(%s, %s)" % (ADJ[t], e, tz), False)
        yield ("date", ADJ[t], "%s(%s)" % (ADJ[t], e), False)
    for (td, d), (tt, tm) in itertools.product(DAT[::2], TIM[::2]):
        yield ("date", "dateTime", "dateTime(%s, %s)" % (d, tm), False)
    for t, e in GTYPES:
        yield ("date", "g", "string(%s)" % e, False)


def date_arith():
    for t, e in DAT + DT + TIM:
        for td, d in DUR:
            for op in ("+", "-"):
                yield ("datearith", t + "-" + op,
                       "%s %s %s" % (p(e), op, p(d)),
                       not arith_valid(op, t, td))
            if arith_valid("+", td, t):
                yield ("datearith", "dur+" + t, "%s + %s" % (p(d), p(e)),
                       False)
    for pool in (DAT, DT, TIM):
        for (ta, a), (tb, b) in itertools.product(pool, pool):
            yield ("datearith", ta + "-diff", "%s - %s" % (p(a), p(b)), False)
    for e in ["xs:date('2000-02-29') + xs:yearMonthDuration('P1Y')",
              "xs:date('2000-01-31') + xs:yearMonthDuration('P1M')",
              "xs:date('2000-03-31') - xs:yearMonthDuration('P1M')",
              "xs:date('9999-12-31') + xs:dayTimeDuration('P1D')",
              "xs:date('-9999-01-01') - xs:dayTimeDuration('P1D')",
              "xs:date('0001-01-01') - xs:dayTimeDuration('P1D')",
              "xs:dateTime('2000-01-01T00:00:00') - "
              "xs:dateTime('2000-01-01T00:00:00Z')",
              "xs:time('23:59:59') + xs:dayTimeDuration('PT1S')",
              "xs:time('00:00:00') - xs:dayTimeDuration('PT1S')",
              "xs:date('2000-01-01') + xs:dayTimeDuration('PT1S')",
              "xs:date('2000-01-01') + xs:dayTimeDuration('-PT1S')",
              "xs:dateTime('1972-06-30T23:59:59Z') + "
              "xs:dayTimeDuration('PT1S')"]:
        yield ("datearith", "edge", e, False)


def duration():
    nums = ["0", "-0.0", "1.5", "xs:double('INF')", "xs:double('NaN')",
            "2", "1e300", "-1", "0e0", "xs:float('1E-45')"]
    for (ta, a), (tb, b) in itertools.product(DUR, DUR):
        for op in ("+", "-", "div"):
            yield ("duration", "%s%s%s" % (ta, op, tb),
                   "%s %s %s" % (p(a), op, p(b)), not arith_valid(op, ta, tb))
        for op in ("eq", "lt"):
            yield ("duration", "%s-%s-%s" % (ta, op, tb),
                   "%s %s %s" % (p(a), op, p(b)), not cmp_valid(op, ta, tb))
    for t, d in DUR:
        for n in nums:
            for op in ("*", "div"):
                yield ("duration", t + op + "num",
                       "%s %s %s" % (p(d), op, n), not arith_valid(op, t, "dec"))
        for f in ("years-from-duration", "months-from-duration",
                  "days-from-duration", "hours-from-duration",
                  "minutes-from-duration", "seconds-from-duration",
                  "string"):
            yield ("duration", f, "%s(%s)" % (f, d), False)


AGG_POOL = ["1", "2.5", "0e0", "-0e0", "xs:float('1.5')", "xs:double('NaN')",
            "xs:double('INF')", "xs:double('-INF')", "9223372036854775807",
            "-1", "xs:untypedAtomic('3')", "0.1", "xs:float('NaN')",
            "xs:untypedAtomic('abc')"]
AGG_FNS = ["sum(%s)", "avg(%s)", "min(%s)", "max(%s)",
           "count(distinct-values(%s))",
           "for $x in distinct-values(%s) order by string($x) return $x",
           "sum(%s, 'empty')"]


def aggregate():
    seqs = []
    for a, b in itertools.product(AGG_POOL, AGG_POOL):
        seqs.append("(%s, %s)" % (a, b))
    for i, (a, b, c) in enumerate(itertools.combinations(AGG_POOL, 3)):
        if i % 3 == 0:
            seqs.append("(%s, %s, %s)" % (a, b, c))
    seqs += ["()", "(1, 1.0, 1e0, xs:float(1))",
             "(xs:yearMonthDuration('P1Y'), xs:yearMonthDuration('-P13M'))",
             "(xs:dayTimeDuration('PT1S'), xs:dayTimeDuration('P1D'))",
             "(xs:yearMonthDuration('P1Y'), xs:dayTimeDuration('P1D'))",
             "('b', 'a', 'B')", "('a', 1)", "(xs:date('2000-01-01'),"
             " xs:date('1999-12-31+14:00'))",
             "(xs:untypedAtomic('1'), xs:untypedAtomic('2'))",
             "(9223372036854775807, 1)", "(9223372036854775807, 1.0)",
             "(9223372036854775807, -9223372036854775807, 1)",
             "(true(), false())", "(xs:anyURI('b'), 'a')",
             "(xs:hexBinary('00'), xs:hexBinary('00'))",
             "(fn:QName('u', 'a'), fn:QName('u', 'p:a'))",
             "(0e0, -0e0)", "(-0e0, 0e0)", "(1, xs:double('NaN'), 0)",
             "(xs:float('NaN'), xs:double('NaN'))",
             "(1, 2, 3, 4)", "(1.1, 2.2, 3.3)", "(1e0, 2, 3.5)"]
    for s in seqs:
        for f in AGG_FNS:
            yield ("aggregate", f.split("(", 1)[0], _fmt(f, s), False)


def qname():
    qnames = ["fn:QName('', 'a')", "fn:QName('http://e', 'p:a')",
              "fn:QName('http://e', 'a')", "fn:QName('', 'p:a')",
              "fn:QName('http://e', '')", "fn:QName('http://e', '1a')",
              "fn:QName('http://e', 'xml:a')",
              "fn:QName('http://www.w3.org/XML/1998/namespace', 'xml:a')",
              "fn:QName('http://e', 'p:a:b')", "fn:QName((), 'a')",
              "fn:QName('http://e', 'é')"]
    for q in qnames:
        q = q.replace("é", "&#xE9;")
        for f in ("local-name-from-QName", "prefix-from-QName",
                  "namespace-uri-from-QName", "string"):
            yield ("qname", f, "%s(%s)" % (f, q), False)
        for q2 in qnames[:3]:
            yield ("qname", "eq", "%s eq %s" % (q, q2), False)
    for rel in ["", "a", "../a", "/a", "?q", "#f", "http://x/y", "..",
                "//h/p", "a b", "../../../../g", "g;x=1/../y", "%zz"]:
        for base in ["http://a/b/c/d;p?q", "file:///tmp/x", "urn:isbn:1",
                     "http://a", "http://a/b/", ""]:
            yield ("qname", "resolve-uri", "resolve-uri(%s, %s)" % (
                xq_string(rel), xq_string(base)), False)
    for e in ["resolve-QName('p:a', <e xmlns:p='u'/>)",
              "resolve-QName('a', <e xmlns='u'/>)",
              "resolve-QName('q:a', <e/>)", "resolve-QName('', <e/>)",
              "resolve-QName('xml:a', <e/>)", "resolve-QName((), <e/>)",
              "in-scope-prefixes(<e xmlns:p='u'/>)",
              "namespace-uri-for-prefix('p', <e xmlns:p='u'/>)",
              "namespace-uri-for-prefix('', <e xmlns='u'/>)",
              "namespace-uri-for-prefix('xml', <e/>)",
              "xs:anyURI('a') eq 'a'", "xs:anyURI('a') lt xs:anyURI('b')",
              "string-length(xs:anyURI(' a '))",
              "xs:anyURI('http://a/b c') eq 'http://a/b c'"]:
        yield ("qname", "edge", e, False)


def boolean():
    for t, e in ALL:
        if t == "str" and len(e) > 100:
            continue
        static_err = t not in NUMERIC | {"str", "ua", "uri", "bool"}
        yield ("boolean", t, "boolean(%s)" % e, static_err)
        yield ("boolean", t + "-not", "not(%s)" % e, static_err)
    for e in ["boolean(())", "boolean((1, 2))", "boolean((<a/>, 1))",
              "boolean((1, <a/>))", "boolean(<a/>)", "boolean('false')",
              "true() and xs:double('NaN')", "false() or ''",
              "xs:untypedAtomic('false') = false()",
              "xs:untypedAtomic('0') eq false()", "true() eq 1",
              "true() lt false()", "false() lt true()"]:
        yield ("boolean", "edge", e, "eq 1" in e or "eq false" in e)


FAMILIES = [arith, unary, typeerr, valcmp, gencmp, casts, lexcast,
            string_fns, string_pairs, numfn, date_fns, date_arith, duration,
            aggregate, qname, boolean]


# Solo cases cost two process starts each (Saxon's JVM start dominates), so
# outside the dedicated families only the first few per family/sub run.
SOLO_CAP = 4
SOLO_FAMILIES = {"typeerr": 1000, "lexcast": 1000}


def _cap_solo(gen):
    seen = {}
    for family, sub, expr, solo in gen:
        if solo:
            cap = SOLO_FAMILIES.get(family, SOLO_CAP)
            n = seen.get((family, sub), 0)
            if n >= cap:
                continue
            seen[(family, sub)] = n + 1
        yield family, sub, expr, solo


def cases():
    """All boundary cases, in a fixed order."""
    out = []
    counters = {}
    for fam in FAMILIES:
        out.extend(number_cases(_cap_solo(fam()), counters))
    return out


if __name__ == "__main__":
    import collections
    import sys
    cs = cases()
    fams = collections.Counter(c["id"].split("/", 1)[0] for c in cs)
    solo = sum(c["solo"] for c in cs)
    for f, n in sorted(fams.items()):
        print("%-10s %6d" % (f, n))
    print("total %d, solo %d" % (len(cs), solo), file=sys.stderr)
