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

"""Property tests: seeded random XQuery 3.0 expressions that must return
true() on a conforming processor. They check invariants within one
processor (round trips, inverses, idempotence, stability) and, where Python
computes the answer exactly, compare against that oracle.

The same seed generates the same cases, so a failure reproduces with
  difftest.py run --set property --seed <seed> --filter <case id> ...

Families (first id segment): xmltree, castrt, number, string, serialize,
sequence, sort, datetime, arith.
"""

import datetime
import decimal
import math
import random
import struct
import unicodedata

from xqgen import number_cases, xq_string, xq_text

# Characters for generated strings: all are XML 1.0 characters whose
# Unicode properties (case mapping, normalization) have been stable for
# many Unicode versions, so Python's unicodedata is a valid oracle.
CHAR_POOL = (list("abcxyzABCXYZ0129 ,.;-_'\"<>&{}[]()*+?|^$\\/=#%!~`@") +
             ["\t", "\n", "\r", "é", "ß", "ü", "Å",
              "Α", "σ", "Σ", "中", "́", "̈",
              "̧", "\U0001F600", "\U00010348", "\U0001D11E", "�",
              "", "퟿", " ", " ", "\u0085", "‮",
              "ﬁ", "Å", "ẛ", "İ", "ŉ"])
# Characters for which upper/lower-case oracles are safe (full case
# mapping, no special casing).
CASE_POOL = list("abcxyzABCXYZ0129 ,.") + ["é", "ü", "Å",
                                          "Α", "σ", "中"]
NAME_POOL = ["a", "b", "c", "d-e", "_f", "g.h", "él", "p:i", "q:j"]
NAMESPACES = {"p": "http://example.org/p", "q": "urn:q"}


def rand_string(rng, pool=CHAR_POOL, maxlen=12):
    return "".join(rng.choice(pool) for _ in range(rng.randint(0, maxlen)))


def seq(items):
    return "(%s)" % ", ".join(items)


# --------------------------------------------------------------------------
# XML trees
# --------------------------------------------------------------------------

def rand_tree(rng, depth=0):
    """A direct element constructor (XQuery text) for a random tree."""
    name = rng.choice(NAME_POOL)
    attrs = []
    used = set()
    nsdecl = ""
    if depth == 0:
        nsdecl = "".join(' xmlns:%s="%s"' % kv
                         for kv in sorted(NAMESPACES.items()))
        if rng.random() < 0.3:
            nsdecl += ' xmlns="http://example.org/default"'
    for _ in range(rng.randint(0, 3)):
        an = rng.choice(["x", "y", "z", "p:w", "xml:lang", "q:v"])
        if an in used:
            continue
        used.add(an)
        attrs.append(' %s="%s"' % (an, xq_text(rand_string(rng), attr=True)))
    children = []
    if depth < 3:
        for _ in range(rng.randint(0, 4)):
            r = rng.random()
            if r < 0.35:
                children.append(rand_tree(rng, depth + 1))
            elif r < 0.7:
                children.append(xq_text(rand_string(rng)))
            elif r < 0.8:
                txt = rand_string(rng, list("ab <&>\t"), 6)
                children.append("<![CDATA[%s]]>" % txt)
            elif r < 0.9:
                txt = rand_string(rng, list("ab <&>"), 6).replace("-", "")
                children.append("<!--%s-->" % xq_text(txt))
            else:
                txt = rand_string(rng, list("ab <&"), 6)
                children.append("<?pi %s?>" % xq_text(txt))
    return "<%s%s%s>%s</%s>" % (name, nsdecl, "".join(attrs),
                                "".join(children), name)


def xmltree(rng, n):
    for _ in range(n):
        t = rand_tree(rng)
        yield ("xmltree", "parse-serialize",
               "let $x := %s return deep-equal(parse-xml(serialize($x))/*, $x)"
               % t)
        yield ("xmltree", "serialize-idempotent",
               "let $x := %s return serialize(parse-xml(serialize($x))/*) eq "
               "serialize($x)" % t)
        yield ("xmltree", "string-value",
               "let $x := %s return string($x) eq string-join($x//text(), '')"
               % t)
        yield ("xmltree", "copy",
               "let $x := %s return deep-equal(element {node-name($x)} "
               "{$x/@*, $x/node()}, $x)" % t)
        yield ("xmltree", "node-count",
               "let $x := %s return count($x//node()) eq "
               "count(parse-xml(serialize($x))/*//node())" % t)
        yield ("xmltree", "names",
               "let $x := %s return every $e in $x/descendant-or-self::* "
               "satisfies namespace-uri($e) eq "
               "namespace-uri-from-QName(node-name($e))" % t)


# --------------------------------------------------------------------------
# Cast round trips
# --------------------------------------------------------------------------

def rand_double(rng):
    while True:
        bits = rng.getrandbits(64)
        v = struct.unpack("<d", struct.pack("<Q", bits))[0]
        if math.isfinite(v):
            if rng.random() < 0.3:  # also plain-sized numbers
                v = rng.uniform(-1e6, 1e6)
            return v


def rand_float32(rng):
    while True:
        bits = rng.getrandbits(32)
        v = struct.unpack("<f", struct.pack("<I", bits))[0]
        if math.isfinite(v):
            return v


def float32_repr(v):
    """Shortest decimal string that rounds to the float32 v."""
    for prec in range(1, 10):
        s = "%.*g" % (prec, v)
        if struct.unpack("<f", struct.pack("<f", float(s)))[0] == v:
            return s
    return "%.9g" % v


def exact_decimal(v):
    """The exact decimal expansion of a binary float."""
    d = decimal.Decimal(v)
    s = format(d, "f")
    return s


def canonical_decimal(d):
    """xs:decimal canonical form (F&O 19.1.2.1 / XSD): no exponent, no
    trailing fractional zeros, at least one digit before the point, and no
    decimal point for integral values."""
    s = format(d, "f")
    if "." in s:
        s = s.rstrip("0").rstrip(".")
    if s in ("-0", ""):
        s = "0"
    return s


def castrt(rng, n):
    for _ in range(n):
        v = rand_double(rng)
        r = repr(v)
        yield ("castrt", "double-string",
               "let $d := xs:double(%s) return xs:double(string($d)) eq $d"
               % xq_string(r))
        yield ("castrt", "double-exact",
               "xs:double(%s) eq xs:double(%s)" % (
                   xq_string(exact_decimal(v)), xq_string(r)))
        yield ("castrt", "double-17g",
               "xs:double(%s) eq xs:double(%s)" % (
                   xq_string("%.17e" % v), xq_string(r)))
        f = rand_float32(rng)
        fr = float32_repr(f)
        yield ("castrt", "float-string",
               "let $f := xs:float(%s) return xs:float(string($f)) eq $f"
               % xq_string(fr))
        yield ("castrt", "float-exact",
               "xs:float(%s) eq xs:float(%s)" % (
                   xq_string(exact_decimal(f)), xq_string(fr)))
        yield ("castrt", "float-via-double",
               "xs:float(xs:double(%s)) eq xs:float(%s)" % (
                   xq_string(fr), xq_string(fr)))
        digits = rng.randint(1, 25)
        scale = rng.randint(0, digits)
        iv = rng.getrandbits(digits * 4) % (10 ** digits)
        d = decimal.Decimal(iv).scaleb(-scale) * rng.choice([1, -1])
        ds = format(d, "f")
        yield ("castrt", "decimal-canonical",
               "string(xs:decimal(%s)) eq %s" % (
                   xq_string(ds), xq_string(canonical_decimal(d))))
        yield ("castrt", "decimal-string",
               "let $v := xs:decimal(%s) return xs:decimal(string($v)) eq $v"
               % xq_string(ds))
        i = rng.randint(-2 ** 63, 2 ** 63 - 1)
        padded = ("+" if i >= 0 and rng.random() < 0.3 else "") + \
            ("-" if i < 0 else "") + "0" * rng.randint(0, 3) + str(abs(i))
        yield ("castrt", "integer-canonical",
               "string(xs:integer(%s)) eq %s" % (xq_string(padded),
                                                 xq_string(str(i))))
        yield ("castrt", "integer-untyped",
               "xs:integer(xs:untypedAtomic(%s)) eq xs:integer(%s)" % (
                   xq_string(str(i)), xq_string(str(i))))
        dt = rand_datetime(rng)
        s = dt_lexical(dt)
        yield ("castrt", "dateTime-string",
               "let $v := xs:dateTime(%s) return xs:dateTime(string($v)) eq $v"
               % xq_string(s))
        yield ("castrt", "date-untyped",
               "let $v := xs:date(%s) return xs:date(xs:untypedAtomic("
               "string($v))) eq $v" % xq_string(s[:10] + s[19:].lstrip(
                   ".0123456789")))
        secs = rng.randint(-10 ** 9, 10 ** 9)
        yield ("castrt", "dayTimeDuration-canonical",
               "string(xs:dayTimeDuration(%s)) eq %s" % (
                   xq_string("PT%dS" % secs if secs >= 0 else
                             "-PT%dS" % -secs),
                   xq_string(dtd_canonical(secs))))
        months = rng.randint(-10 ** 5, 10 ** 5)
        yield ("castrt", "yearMonthDuration-canonical",
               "string(xs:yearMonthDuration(%s)) eq %s" % (
                   xq_string("P%dM" % months if months >= 0 else
                             "-P%dM" % -months),
                   xq_string(ymd_canonical(months))))
        bl = xq_string(rng.choice(["1", "0", "true", "false"]))
        yield ("castrt", "boolean",
               "xs:boolean(string(xs:boolean(%s))) eq xs:boolean(%s)"
               % (bl, bl))
        hx = "".join(rng.choice("0123456789abcdefABCDEF")
                     for _ in range(2 * rng.randint(0, 8)))
        yield ("castrt", "binary",
               "xs:hexBinary(xs:base64Binary(xs:hexBinary(%s))) eq "
               "xs:hexBinary(%s)" % (xq_string(hx), xq_string(hx)))


def dtd_canonical(secs):
    sign = "-" if secs < 0 else ""
    s = abs(secs)
    if s == 0:
        return "PT0S"
    d, s = divmod(s, 86400)
    h, s = divmod(s, 3600)
    m, s = divmod(s, 60)
    out = sign + "P"
    if d:
        out += "%dD" % d
    if h or m or s:
        out += "T"
        if h:
            out += "%dH" % h
        if m:
            out += "%dM" % m
        if s:
            out += "%dS" % s
    return out


def ymd_canonical(months):
    sign = "-" if months < 0 else ""
    y, m = divmod(abs(months), 12)
    if y == 0 and m == 0:
        return "P0M"
    out = sign + "P"
    if y:
        out += "%dY" % y
    if m:
        out += "%dM" % m
    return out


# --------------------------------------------------------------------------
# Dates and times
# --------------------------------------------------------------------------

def rand_tz(rng):
    """Returns tz offset in minutes, or None for no timezone."""
    r = rng.random()
    if r < 0.2:
        return None
    if r < 0.4:
        return 0
    return rng.choice([-1, 1]) * rng.choice([0, 30, 45, 60, 330, 600, 720,
                                             840, rng.randint(0, 840)])


def tz_lexical(tz):
    if tz is None:
        return ""
    if tz == 0:
        return "Z"
    sign = "-" if tz < 0 else "+"
    return "%s%02d:%02d" % (sign, abs(tz) // 60, abs(tz) % 60)


def rand_datetime(rng):
    y = rng.choice([rng.randint(1, 9999), rng.randint(1890, 2100)])
    base = datetime.datetime(y, 1, 1)
    end = datetime.datetime(min(y + 1, 9999), 1, 1) if y < 9999 else \
        datetime.datetime(9999, 12, 31, 23, 59, 59)
    span = int((end - base).total_seconds())
    dt = base + datetime.timedelta(seconds=rng.randint(0, max(span - 1, 0)))
    micro = rng.choice([0, 0, rng.randint(0, 999999)])
    return (dt.replace(microsecond=micro), rand_tz(rng))


def dt_lexical(dtz):
    dt, tz = dtz
    s = "%04d-%02d-%02dT%02d:%02d:%02d" % (dt.year, dt.month, dt.day,
                                          dt.hour, dt.minute, dt.second)
    if dt.microsecond:
        s += (".%06d" % dt.microsecond).rstrip("0")
    return s + tz_lexical(tz)


def datetimes(rng, n):
    for _ in range(n):
        dtz = rand_datetime(rng)
        dt, tz = dtz
        lex = dt_lexical(dtz)
        d = xq_string(lex)
        date_lex = xq_string("%04d-%02d-%02d" % (dt.year, dt.month, dt.day))
        # Week numbering in the default calendar is implementation-defined
        # (Zorba's AD calendar starts weeks on Sunday); the ISO calendar
        # defines it (F&O 3.0 9.8.4.1).
        yield ("datetime", "weekday",
               "format-date(xs:date(%s), '[F1]', (), 'ISO', ()) eq '%d'" % (
                   date_lex, dt.isoweekday()))
        yield ("datetime", "day-in-year",
               "format-date(xs:date(%s), '[d]') eq '%d'" % (
                   date_lex, dt.timetuple().tm_yday))
        yield ("datetime", "iso-week",
               "format-date(xs:date(%s), '[W]', (), 'ISO', ()) eq '%d'" % (
                   date_lex, dt.isocalendar()[1]))
        yield ("datetime", "format-roundtrip",
               "let $d := xs:dateTime(%s) return xs:dateTime(format-dateTime("
               "$d, '[Y0001]-[M01]-[D01]T[H01]:[m01]:[s01].[f000001]%s')) eq $d"
               % (d, "[Z]" if tz is not None else ""))
        yield ("datetime", "components",
               "let $d := xs:dateTime(%s) return deep-equal(("
               "year-from-dateTime($d), month-from-dateTime($d), "
               "day-from-dateTime($d), hours-from-dateTime($d), "
               "minutes-from-dateTime($d)), (%d, %d, %d, %d, %d))"
               % (d, dt.year, dt.month, dt.day, dt.hour, dt.minute))
        days = rng.randint(-3000, 3000)
        try:
            later = dt + datetime.timedelta(days=days)
        except OverflowError:
            later = None
        if later and 1 <= later.year <= 9999:
            yield ("datetime", "plus-days",
                   "xs:date(%s) + xs:dayTimeDuration('%sP%dD') eq "
                   "xs:date('%04d-%02d-%02d')" % (
                       date_lex, "-" if days < 0 else "", abs(days),
                       later.year, later.month, later.day))
        dtz2 = rand_datetime(rng)
        if tz is not None and dtz2[1] is not None:
            a = dt - datetime.timedelta(minutes=tz)
            b = dtz2[0] - datetime.timedelta(minutes=dtz2[1])
            diff = a - b
            us = diff.days * 86400 * 10 ** 6 + diff.seconds * 10 ** 6 + \
                diff.microseconds
            yield ("datetime", "difference",
                   "xs:dateTime(%s) - xs:dateTime(%s) eq "
                   "xs:dayTimeDuration('%s')" % (
                       d, xq_string(dt_lexical(dtz2)), dtd_micro(us)))
            yield ("datetime", "difference-inverse",
                   "let $a := xs:dateTime(%s) let $b := xs:dateTime(%s) "
                   "return $b + ($a - $b) eq $a" % (
                       d, xq_string(dt_lexical(dtz2))))
        tz2 = rng.choice([0, 840, -840, 330, -600, 45])
        # Adjusting a dateTime without a timezone attaches the timezone
        # instead of converting, so these hold only with a timezone.
        if tz is not None:
            yield ("datetime", "adjust-same-instant",
                   "let $d := xs:dateTime(%s) return adjust-dateTime-to-"
                   "timezone($d, xs:dayTimeDuration('%s')) eq $d"
                   % (d, dtd_micro(tz2 * 60 * 10 ** 6)))
            yield ("datetime", "adjust-twice",
                   "let $d := xs:dateTime(%s) let $tz := xs:dayTimeDuration("
                   "'%s') return deep-equal(adjust-dateTime-to-timezone("
                   "adjust-dateTime-to-timezone($d, xs:dayTimeDuration("
                   "'PT1H')), $tz), adjust-dateTime-to-timezone($d, $tz))"
                   % (d, dtd_micro(tz2 * 60 * 10 ** 6)))
        if tz is not None and 2 <= dt.year <= 9998:
            u = dt - datetime.timedelta(minutes=tz)
            yield ("datetime", "adjust-utc-string",
                   "string(adjust-dateTime-to-timezone(xs:dateTime(%s), "
                   "xs:dayTimeDuration('PT0S'))) eq %s" % (
                       d, xq_string(dt_lexical((u, 0)))))
        yield ("datetime", "date-from-dateTime",
               "xs:date(xs:dateTime(%s)) eq xs:date(%s)" % (
                   d, xq_string(lex[:10] + tz_lexical(tz))))


def dtd_micro(us):
    """A canonical xs:dayTimeDuration lexical form for us microseconds."""
    sign = "-" if us < 0 else ""
    secs, frac = divmod(abs(us), 10 ** 6)
    if secs == 0 and frac == 0:
        return "PT0S"
    d, rest = divmod(secs, 86400)
    h, rest = divmod(rest, 3600)
    m, sec = divmod(rest, 60)
    out = sign + "P" + ("%dD" % d if d else "")
    if h or m or sec or frac:
        out += "T" + ("%dH" % h if h else "") + ("%dM" % m if m else "")
        if sec or frac:
            out += "%d%sS" % (sec, (".%06d" % frac).rstrip("0") if frac
                              else "")
    return out


# --------------------------------------------------------------------------
# Strings
# --------------------------------------------------------------------------

def strings(rng, n):
    for _ in range(n):
        s = rand_string(rng)
        lit = xq_string(s)
        yield ("string", "codepoints",
               "codepoints-to-string(string-to-codepoints(%s)) eq %s"
               % (lit, lit))
        yield ("string", "length",
               "string-length(%s) eq %d" % (lit, len(s)))
        yield ("string", "tokenize-join",
               "string-join(tokenize(%s, ','), ',') eq %s" % (lit, lit)
               if s else "empty(tokenize('', ','))")
        k = rng.randint(0, len(s) + 1)
        yield ("string", "substring-split",
               "concat(substring(%s, 1, %d), substring(%s, %d)) eq %s"
               % (lit, k, lit, k + 1, lit))
        a = rng.choice([-1.5, 0, 0.5, 1, 1.5, 2, 2.5, 3, len(s), len(s) + 2])
        ln = rng.choice([0, 0.5, 1, 1.49, 1.5, 2, 3, 100])
        yield ("string", "substring-oracle",
               "substring(%s, %r, %r) eq %s" % (
                   lit, a, ln, xq_string(substring_oracle(s, a, ln))))
        yield ("string", "reverse-twice",
               "codepoints-to-string(reverse(reverse(string-to-codepoints(%s))))"
               " eq %s" % (lit, lit))
        for form in ("NFC", "NFD", "NFKC", "NFKD"):
            yield ("string", "normalize-" + form,
                   "normalize-unicode(%s, '%s') eq %s" % (
                       lit, form, xq_string(unicodedata.normalize(form, s))))
        yield ("string", "normalize-idempotent",
               "let $n := normalize-unicode(%s) return "
               "normalize-unicode($n) eq $n" % lit)
        yield ("string", "normalize-space-oracle",
               "normalize-space(%s) eq %s" % (
                   lit, xq_string(" ".join(
                       x for x in s.replace("\t", " ").replace("\n", " ")
                       .replace("\r", " ").split(" ") if x))))
        t = rand_string(rng)
        tl = xq_string(t)
        yield ("string", "concat-contains",
               "contains(concat(%s, %s), %s) and starts-with(concat(%s, %s), "
               "%s) and ends-with(concat(%s, %s), %s)"
               % (lit, tl, tl, lit, tl, lit, lit, tl, tl))
        yield ("string", "compare-antisymmetric",
               "compare(%s, %s) eq -compare(%s, %s)" % (lit, tl, tl, lit))
        cmp = (s > t) - (s < t)
        yield ("string", "compare-codepoint",
               "compare(%s, %s) eq %d" % (lit, tl, cmp))
        c = rand_string(rng, CASE_POOL)
        yield ("string", "case-oracle",
               "upper-case(%s) eq %s and lower-case(%s) eq %s" % (
                   xq_string(c), xq_string(c.upper()), xq_string(c),
                   xq_string(c.lower())))
        yield ("string", "translate-identity",
               "translate(%s, 'abc', 'abc') eq %s" % (lit, lit))
        yield ("string", "encode-for-uri-oracle",
               "encode-for-uri(%s) eq %s" % (lit, xq_string(
                   encode_for_uri(s))))


def substring_oracle(s, start, length):
    """F&O 5.4.3: characters at positions p with
    round(start) <= p < round(start) + round(length)."""
    def rnd(x):
        return math.floor(x + 0.5)
    lo = rnd(start)
    hi = lo + rnd(length)
    return "".join(ch for p, ch in enumerate(s, 1) if lo <= p < hi)


def encode_for_uri(s):
    out = []
    for ch in s:
        if ch.isascii() and (ch.isalnum() or ch in "-_.~"):
            out.append(ch)
        else:
            out.extend("%%%02X" % b for b in ch.encode("utf-8"))
    return "".join(out)


def serialize_rt(rng, n):
    for _ in range(n):
        s = rand_string(rng)
        lit = xq_string(s)
        yield ("serialize", "text",
               "parse-xml(serialize(<a>{%s}</a>))/a/string() eq %s"
               % (lit, lit))
        yield ("serialize", "attribute",
               "parse-xml(serialize(<a b='{%s}'/>))/a/@b/string() eq %s"
               % (lit, lit))
        # A CR in a comment cannot be escaped and is normalized to LF when
        # parsed again.
        yield ("serialize", "comment",
               "let $c := %s return if (contains($c, '--') or ends-with($c, "
               "'-') or contains($c, codepoints-to-string(13))) then true() else parse-xml(serialize(<a>{comment {$c}}"
               "</a>))/a/comment()/string() eq $c" % lit)
        yield ("serialize", "text-node-string",
               "string(text {%s}) eq %s" % (lit, lit))


# --------------------------------------------------------------------------
# Sequences, sorting, arithmetic
# --------------------------------------------------------------------------

def rand_ints(rng, n=None, lo=-50, hi=50):
    return [rng.randint(lo, hi) for _ in range(
        rng.randint(0, 12) if n is None else n)]


def sequences(rng, n):
    for _ in range(n):
        xs = rand_ints(rng)
        s = seq(str(x) for x in xs)
        yield ("sequence", "reverse-twice",
               "deep-equal(reverse(reverse(%s)), %s)" % (s, s))
        yield ("sequence", "reverse-oracle",
               "deep-equal(reverse(%s), %s)" % (
                   s, seq(str(x) for x in reversed(xs))))
        yield ("sequence", "distinct-idempotent",
               "deep-equal(distinct-values(distinct-values(%s)), "
               "distinct-values(%s))" % (s, s))
        yield ("sequence", "distinct-count",
               "count(distinct-values(%s)) eq %d" % (s, len(set(xs))))
        yield ("sequence", "sum-oracle", "sum(%s) eq %d" % (s, sum(xs)))
        yield ("sequence", "sum-reverse",
               "sum(%s) eq sum(reverse(%s))" % (s, s))
        if xs:
            yield ("sequence", "min-max",
                   "min(%s) eq %d and max(%s) eq %d" % (s, min(xs), s,
                                                        max(xs)))
        a = rng.choice([-1, 0, 0.5, 1, 1.5, 2.5, 3, 20, -0.5])
        ln = rng.choice([0, 0.4, 0.5, 1, 2, 2.5, 100])
        yield ("sequence", "subsequence-oracle",
               "deep-equal(subsequence(%s, %r, %r), %s)" % (
                   s, a, ln, seq(str(x) for x in subsequence_oracle(xs, a,
                                                                     ln))))
        pos = rng.randint(-1, len(xs) + 2)
        yield ("sequence", "insert-remove",
               "deep-equal(remove(insert-before(%s, %d, 999), %d), %s)" % (
                   s, pos, min(max(pos, 1), len(xs) + 1), s))
        if xs:
            v = rng.choice(xs)
            yield ("sequence", "index-of-oracle",
                   "deep-equal(index-of(%s, %d), %s)" % (
                       s, v, seq(str(i) for i, x in enumerate(xs, 1)
                                 if x == v)))
        ds = [rng.choice(["1", "1.0", "1e0", "xs:float(1)", "2", "0.5e0",
                          "xs:double('NaN')", "xs:float('NaN')",
                          "xs:untypedAtomic('1')", "'1'", "-0e0", "0"])
              for _ in range(rng.randint(0, 6))]
        mixed = seq(ds)
        yield ("sequence", "distinct-mixed-idempotent",
               "deep-equal(distinct-values(distinct-values(%s)), "
               "distinct-values(%s))" % (mixed, mixed))
        yield ("sequence", "deep-equal-reflexive",
               "deep-equal(%s, %s)" % (mixed, mixed))


def subsequence_oracle(xs, start, length):
    def rnd(x):
        return math.floor(x + 0.5)
    lo = rnd(start)
    hi = lo + rnd(length)
    return [x for p, x in enumerate(xs, 1) if lo <= p < hi]


def sorting(rng, n):
    for _ in range(n):
        keys = rand_ints(rng, rng.randint(0, 15), 0, 4)
        recs = seq("<r k='%d' i='%d'/>" % (k, i)
                   for i, k in enumerate(keys))
        order = [i for i, _ in sorted(enumerate(keys), key=lambda t: t[1])]
        yield ("sort", "stable-ascending",
               "deep-equal(for $r in %s stable order by xs:integer($r/@k) "
               "return xs:integer($r/@i), %s)" % (recs, seq(map(str, order))))
        order_d = [i for i, _ in sorted(enumerate(keys),
                                        key=lambda t: -t[1])]
        yield ("sort", "stable-descending",
               "deep-equal(for $r in %s stable order by xs:integer($r/@k) "
               "descending return xs:integer($r/@i), %s)" % (
                   recs, seq(map(str, order_d))))
        yield ("sort", "unstable-is-sorted",
               "let $s := for $r in %s order by xs:integer($r/@k) return "
               "xs:integer($r/@k) return deep-equal($s, %s)" % (
                   recs, seq(map(str, sorted(keys)))))
        vals = [rng.choice(["1e0", "-1e0", "xs:double('NaN')", "0e0",
                            "xs:double('INF')", "xs:double('-INF')", "2.5e0"])
                for _ in range(rng.randint(0, 8))]
        pyvals = [float(v.replace("xs:double('", "").replace("')", "")
                        .replace("NaN", "nan").replace("INF", "inf"))
                  for v in vals]
        # NaN sorts before all other values with empty least (XQuery 3.0
        # 3.10.8); -0 and 0 are equal.
        asc = sorted(range(len(vals)), key=lambda i: (
            0 if math.isnan(pyvals[i]) else 1,
            0 if math.isnan(pyvals[i]) else pyvals[i]))
        yield ("sort", "nan-least",
               "deep-equal(for $x at $i in %s stable order by $x empty least "
               "return $i, %s)" % (seq(vals), seq(str(i + 1) for i in asc)))
        strs = [rand_string(rng, list("aAbBé\U0001F600 "), 3)
                for _ in range(rng.randint(0, 8))]
        yield ("sort", "codepoint-order",
               "deep-equal(for $s in %s order by $s return $s, %s)" % (
                   seq(xq_string(x) for x in strs),
                   seq(xq_string(x) for x in sorted(strs))))


def arith(rng, n):
    lim = 2 ** 63
    for _ in range(n):
        a = rng.choice([rng.randint(-lim, lim - 1), rng.randint(-1000, 1000)])
        b = rng.choice([rng.randint(-lim, lim - 1), rng.randint(-50, 50)])
        if b == 0:
            b = 7
        q = abs(a) // abs(b) * (1 if (a >= 0) == (b > 0) else -1)
        m = a - q * b
        if -lim <= q < lim:
            yield ("arith", "idiv-mod-identity",
                   "let $a := xs:integer('%d') let $b := xs:integer('%d') "
                   "return ($a idiv $b) * $b + ($a mod $b) eq $a" % (a, b))
            yield ("arith", "idiv-mod-oracle",
                   "xs:integer('%d') idiv xs:integer('%d') eq xs:integer('%d') "
                   "and xs:integer('%d') mod xs:integer('%d') eq "
                   "xs:integer('%d')" % (a, b, q, a, b, m))
        for op, f in (("+", lambda x, y: x + y), ("-", lambda x, y: x - y),
                      ("*", lambda x, y: x * y)):
            r = f(a, b)
            if -lim <= r < lim:
                yield ("arith", "integer" + op,
                       "xs:integer('%d') %s xs:integer('%d') eq "
                       "xs:integer('%d')" % (a, op, b, r))
            else:
                # Zorba raises FOAR0002 on xs:integer overflow (64-bit).
                yield ("arith", "integer-overflow" + op,
                       "try { xs:integer('%d') %s xs:integer('%d') eq "
                       "xs:integer('%d') } catch err:FOAR0002 { true() }"
                       % (a, op, b, r))
        x = decimal.Decimal(rng.randint(-10 ** 12, 10 ** 12)).scaleb(
            -rng.randint(0, 8))
        y = decimal.Decimal(rng.randint(-10 ** 12, 10 ** 12)).scaleb(
            -rng.randint(0, 8))
        ctx = decimal.Context(prec=60)
        for op, f in (("+", ctx.add), ("-", ctx.subtract),
                      ("*", ctx.multiply)):
            yield ("arith", "decimal" + op,
                   "%s %s %s eq %s" % (dec_lit(x), op, dec_lit(y),
                                       dec_lit(f(x, y))))
        if y != 0:
            yield ("arith", "decimal-mod",
                   "(%s mod %s) eq %s" % (dec_lit(x), dec_lit(y),
                                          dec_lit(ctx.remainder(x, y))))
        da, db = rand_double(rng), rand_double(rng)
        yield ("arith", "double-commutative",
               "let $a := xs:double(%s) let $b := xs:double(%s) return "
               "$a + $b eq $b + $a and $a * $b eq $b * $a"
               % (xq_string(repr(da)), xq_string(repr(db))))
        s = da + db
        if math.isfinite(s):
            yield ("arith", "double-add-oracle",
                   "xs:double(%s) + xs:double(%s) eq xs:double(%s)" % (
                       xq_string(repr(da)), xq_string(repr(db)),
                       xq_string(repr(s))))
        pr = da * db
        if math.isfinite(pr):
            yield ("arith", "double-mul-oracle",
                   "xs:double(%s) * xs:double(%s) eq xs:double(%s)" % (
                       xq_string(repr(da)), xq_string(repr(db)),
                       xq_string(repr(pr))))
        if db != 0 and math.isfinite(da / db):
            yield ("arith", "double-div-oracle",
                   "xs:double(%s) div xs:double(%s) eq xs:double(%s)" % (
                       xq_string(repr(da)), xq_string(repr(db)),
                       xq_string(repr(da / db))))
        if db != 0:
            fm = math.fmod(da, db)
            yield ("arith", "double-mod-oracle",
                   "let $r := xs:double(%s) mod xs:double(%s) return $r eq "
                   "xs:double(%s)" % (xq_string(repr(da)), xq_string(repr(db)),
                                      xq_string(repr(fm))))
        r = rng.choice([0.5, 1.5, 2.5, -0.5, -1.5, -2.5, 0.49999999999999994,
                        1.25, -1.25, 2.675, 1e15 + 0.5])
        # The literal is an xs:decimal, so the oracle rounds the decimal
        # value (half up), not the nearest double.
        rd = decimal.Decimal(repr(r))
        yield ("arith", "round-oracle",
               "round(%s) eq %d" % (dec_lit(rd),
                                    math.floor(rd + decimal.Decimal("0.5"))))
        yield ("arith", "round-half-even-oracle",
               "round-half-to-even(%s) eq %s" % (
                   dec_lit(decimal.Decimal(repr(r))),
                   dec_lit(decimal.Decimal(repr(r)).quantize(
                       decimal.Decimal(1), rounding=decimal.ROUND_HALF_EVEN))))


def dec_lit(d):
    s = canonical_decimal(d)
    if s.startswith("-"):
        return "(-%s)" % (s[1:] if "." in s else s[1:] + ".0")
    return s if "." in s else s + ".0"


FAMILIES = [(xmltree, 60), (castrt, 80), (datetimes, 80), (strings, 60),
            (serialize_rt, 80), (sequences, 60), (sorting, 60), (arith, 60)]


def cases(seed, scale=1.0):
    """The property cases for a seed. Each family gets its own generator
    seeded from (seed, family), so adding a family does not change the
    cases of the others."""
    out = []
    counters = {}
    for fam, n in FAMILIES:
        rng = random.Random("%s/%s" % (seed, fam.__name__))
        gen = ((f, sub, expr, False) for f, sub, expr in
               fam(rng, max(1, int(n * scale))))
        out.extend(number_cases(gen, counters))
    return out


if __name__ == "__main__":
    import collections
    import sys
    cs = cases(int(sys.argv[1]) if len(sys.argv) > 1 else 1)
    fams = collections.Counter(c["id"].split("/", 1)[0] for c in cs)
    for f, n in sorted(fams.items()):
        print("%-10s %6d" % (f, n))
    print("total %d" % len(cs))
