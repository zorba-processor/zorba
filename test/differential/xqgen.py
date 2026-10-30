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

"""Shared helpers for the generated query sets (boundary.py, property.py)
and the batch query format the harness runs them in.

A generated case is a dict with
  id     family/sub/nnnn (the first segment is the report family)
  expr   a self-contained XQuery 3.0 expression
  solo   True if the case must run in a query of its own (it is expected to
         raise a static type error, which would abort a whole batch)

Cases run in batches: one query evaluates many cases, each inside
try/catch, and prints one line per case:

  #<n> <type>:<value> <type>:<value> ...    the result items
  #<n> ()                                   the empty sequence
  #<n> !<code>                              a dynamic error

Values are escaped (local:esc) so that every line is printable ASCII and
the item separator (space) is unambiguous.
"""

import re

ERR_NS = "http://www.w3.org/2005/xqt-errors"

PRELUDE = r'''xquery version "3.0";
declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";
declare option output:method "text";

declare function local:t($i as item()) as xs:string {
  typeswitch ($i)
    case xs:untypedAtomic return "untypedAtomic"
    case xs:string return "string"
    case xs:byte return "byte"
    case xs:short return "short"
    case xs:int return "int"
    case xs:long return "long"
    case xs:unsignedByte return "unsignedByte"
    case xs:unsignedShort return "unsignedShort"
    case xs:unsignedInt return "unsignedInt"
    case xs:unsignedLong return "unsignedLong"
    case xs:positiveInteger return "positiveInteger"
    case xs:nonNegativeInteger return "nonNegativeInteger"
    case xs:negativeInteger return "negativeInteger"
    case xs:nonPositiveInteger return "nonPositiveInteger"
    case xs:integer return "integer"
    case xs:decimal return "decimal"
    case xs:float return "float"
    case xs:double return "double"
    case xs:boolean return "boolean"
    case xs:yearMonthDuration return "yearMonthDuration"
    case xs:dayTimeDuration return "dayTimeDuration"
    case xs:duration return "duration"
    case xs:dateTime return "dateTime"
    case xs:date return "date"
    case xs:time return "time"
    case xs:gYearMonth return "gYearMonth"
    case xs:gYear return "gYear"
    case xs:gMonthDay return "gMonthDay"
    case xs:gDay return "gDay"
    case xs:gMonth return "gMonth"
    case xs:hexBinary return "hexBinary"
    case xs:base64Binary return "base64Binary"
    case xs:anyURI return "anyURI"
    case xs:QName return "QName"
    case xs:anyAtomicType return "atomic"
    case document-node() return "document"
    case element() return "element"
    case attribute() return "attribute"
    case text() return "text"
    case comment() return "comment"
    case processing-instruction() return "pi"
    default return "item"
};

declare function local:esc($s as xs:string) as xs:string {
  string-join(
    for $c in string-to-codepoints($s)
    return if ($c gt 32 and $c lt 127 and $c ne 92)
           then codepoints-to-string($c)
           else concat("\", string($c), ";"),
    "")
};

declare function local:show($r as item()*) as xs:string {
  if (empty($r)) then "()"
  else string-join(
    for $i in $r
    return concat(local:t($i), ":",
                  if ($i instance of function(*)) then "fn"
                  else local:esc(string($i))),
    " ")
};

declare function local:err($c as xs:QName) as xs:string {
  concat("!",
    if (namespace-uri-from-QName($c) eq "http://www.w3.org/2005/xqt-errors")
    then local-name-from-QName($c)
    else concat("Q{", namespace-uri-from-QName($c), "}",
                local-name-from-QName($c)))
};
'''


def batch_query(exprs):
    """The query text that evaluates exprs (a list of expressions) and
    prints one line per expression, numbered from 0."""
    parts = []
    for n, expr in enumerate(exprs):
        parts.append('  concat("#%d ", try { local:show((\n%s\n)) } '
                     'catch * { local:err($err:code) })' % (n, expr))
    return PRELUDE + "\nstring-join((\n" + ",\n".join(parts) + \
        "\n), \"&#10;\")\n"


LINE_RE = re.compile(r"^#(\d+) (.*)$")


def parse_batch_output(out, n):
    """Parse the output of batch_query for n expressions. Returns a list of
    n result strings, or None if the output is incomplete or malformed."""
    results = [None] * n
    for line in out.splitlines():
        line = line.rstrip("\r")
        if not line:
            continue
        m = LINE_RE.match(line)
        if not m:
            return None
        i = int(m.group(1))
        if i >= n or results[i] is not None:
            return None
        results[i] = m.group(2)
    if any(r is None for r in results):
        return None
    return results


def xq_string(s):
    """An XQuery string literal for the Python string s. Characters outside
    printable ASCII become character references, so the query text is
    ASCII and line-end normalization cannot change the value."""
    out = ['"']
    for ch in s:
        c = ord(ch)
        if ch == '"':
            out.append('""')
        elif ch == "&":
            out.append("&amp;")
        elif 32 <= c < 127:
            out.append(ch)
        else:
            out.append("&#x%X;" % c)
    out.append('"')
    return "".join(out)


def xq_text(s, attr=False):
    """Escape s for the content of a direct element constructor (or, with
    attr, of a direct attribute value delimited by double quotes)."""
    out = []
    for ch in s:
        c = ord(ch)
        if ch == "{":
            out.append("{{")
        elif ch == "}":
            out.append("}}")
        elif ch == "<":
            out.append("&lt;")
        elif ch == ">":
            out.append("&gt;")
        elif ch == "&":
            out.append("&amp;")
        elif ch == '"' and attr:
            out.append("&quot;")
        elif 32 <= c < 127:
            out.append(ch)
        else:
            out.append("&#x%X;" % c)
    return "".join(out)


def esc_value(s):
    """Python version of local:esc, for expected values in unit tests."""
    return "".join(ch if 32 < ord(ch) < 127 and ch != "\\"
                   else "\\%d;" % ord(ch) for ch in s)


def number_cases(cases, family_counters=None):
    """Assign ids family/sub/nnnn to (family, sub, expr, solo) tuples."""
    counters = family_counters if family_counters is not None else {}
    out = []
    for family, sub, expr, solo in cases:
        key = (family, sub)
        n = counters.get(key, 0)
        counters[key] = n + 1
        out.append({"id": "%s/%s/%04d" % (family, sub, n), "expr": expr,
                    "solo": solo})
    return out
