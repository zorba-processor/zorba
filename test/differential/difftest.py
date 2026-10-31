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

"""Differential testing of Zorba against Saxon-HE.

Three query sets (--set):

  rbkt      the plain XQuery 3.0 queries of the rbkt test suite
            (test/rbkt/Queries/zorba), run on both processors
  boundary  generated boundary-value matrices (boundary.py), run on both
            processors
  property  generated property tests (property.py), run on Zorba only;
            each case must return true()

Generated cases run in batches (xqgen.py): one query evaluates many cases,
each inside try/catch. If a batch does not complete on a processor (a
static error, a crash, a timeout), it is split in halves until the failing
cases run alone, so every case gets its own result.

rbkt and boundary results are classified per query (case):

  agree             same canonical result, or both raised the same error
  zorba-error-only  Zorba raised an error, Saxon returned a result
  saxon-error-only  Saxon raised an error, Zorba returned a result
  different-error-code  both raised errors with different codes
  different-result  both returned results that differ after normalization
  timeout           at least one processor exceeded the per-query timeout
  harness-skip      not run: the query uses features outside the common
                    subset (JSONiq, Zorba modules, full-text, updates,
                    scripting, schema awareness, ...); the reason is recorded
                    (rbkt only)

property results are pass, fail (the invariant returned something other
than true()), error or timeout.

Disagreements are informational. The exit status is non-zero only for
harness errors (a processor that cannot run a trivial query, bad
arguments, missing shard results).

Subcommands:
  select   print the corpus selection (id, reason) as TSV
  run      run one shard and write results.tsv, raw outputs and status.json
  report   merge shard results into summary.md and results.tsv

Python 3 standard library only.
"""

import argparse
import collections
import concurrent.futures
import decimal
import json
import math
import os
import re
import signal
import subprocess
import sys
import threading
import time
import urllib.parse
import xml.etree.ElementTree as ET

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xqgen  # noqa: E402

CLASSES = ["agree", "zorba-error-only", "saxon-error-only",
           "different-error-code", "different-result", "timeout",
           "harness-skip"]
PROPERTY_CLASSES = ["pass", "fail", "error", "timeout"]
SETS = ["rbkt", "boundary", "property"]

ERR_NS = "http://www.w3.org/2005/xqt-errors"
OUTPUT_NS = "http://www.w3.org/2010/xslt-xquery-serialization"

# Directories under Queries/zorba that test Zorba-specific features
# (modules, store, JSONiq, scripting, updates, full-text, schema awareness,
# HTTP, test-driver features). Queries in other directories are still
# filtered by content below.
EXCLUDED_DIRS = {
    "annotations": "zorba-specific",
    "caching": "zorba-specific",
    "collections": "collections",
    "csv": "zorba-module",
    "dctx": "zorba-specific",
    "debugger": "zorba-specific",
    "eval": "zorba-specific",
    "ext_var": "external-variable",
    "extern": "zorba-specific",
    "extern-func": "zorba-specific",
    "fetch": "zorba-module",
    "file": "zorba-module",
    "fulltext": "full-text",
    "http-client": "http",
    "image": "zorba-module",
    "index": "zorba-specific",
    "json": "jsoniq",
    "jsoniq": "jsoniq",
    "jsound": "jsoniq",
    "modules": "module-import",
    "no-copy": "zorba-specific",
    "nondeterministic": "nondeterministic",
    "pul-schema": "updates",
    "random": "nondeterministic",
    "reflection": "zorba-specific",
    "schemas": "schema-aware",
    "scripting": "scripting",
    "block": "scripting",
    "store": "zorba-specific",
    "testdriver": "zorba-specific",
    "updates": "updates",
    "versioning": "module-import",
    "xdm-schema": "schema-aware",
    "xqddf": "collections",
    "xqdoc": "zorba-module",
    "xqueryx": "zorba-specific",
    "xray": "zorba-specific",
    "zorba-query": "zorba-module",
}

# Content rules, checked in order; the first match gives the skip reason.
# Comments are stripped before matching.
CONTENT_RULES = [
    ("jsoniq", re.compile(r"\bjsoniq\s+version\b|http://jsoniq\.org/|"
                          r"\b(?:jn|js|jerr):[A-Za-z]|"
                          # JSON object constructors
                          r"\{\||\{\s*\"[^\"\n]*\"\s*:(?!=)")),
    ("xquery-3.1", re.compile(r"\bxquery\s+version\s+[\"']3\.1[\"']")),
    ("zorba-specific", re.compile(
        r"http://zorba\.io/|http://www\.zorba-xquery\.com/|"
        r"http://expath\.org/|http://www\.28msec\.com/|"
        r"%\s*(?:an|zann):|\b(?:zerr|zwarn):[A-Za-z]|"
        r"\A\s*#!")),  # shebang line, a Zorba extension
    ("full-text", re.compile(
        r"\bcontains\s+text\b|\bftcontains\b|\bftand\b|\bftor\b|"
        r"\busing\s+(?:stemming|thesaurus|stop\s+words)\b")),
    ("updates", re.compile(
        r"\binsert\s+nodes?\b|\bdelete\s+nodes?\b|"
        r"\breplace\s+(?:value\s+of\s+)?node\b|\brename\s+node\b|"
        r"\bcopy\s+\$[\w.-]+\s*:=|\bdeclare\s+updating\b|%\s*updating\b|"
        r"\bfn:put\s*\(|\bput\s*\(")),
    ("scripting", re.compile(
        r"\bexit\s+returning\b|\bwhile\s*\(|\bbreak\s+loop\b|"
        r"\bcontinue\s+loop\b|\bdeclare\s+sequential\b|"
        r"(?:^|[;{])\s*variable\s+\$|[;{]\s*\$[\w.-]+\s*:=",
        re.M)),
    ("schema-aware", re.compile(
        r"\bimport\s+schema\b|\bvalidate\s*(?:lax|strict|type|\{)|"
        r"\bschema-(?:element|attribute)\s*\(")),
    ("module-import", re.compile(r"\bimport\s+module\b")),
    ("library-module", re.compile(r"^\s*module\s+namespace\b", re.M)),
    ("collections", re.compile(r"\b(?:fn:)?(?:uri-)?collection\s*\(")),
    ("http", re.compile(r"\$HTTP_TEST_SERVER|zorbatest|"
                        r"(?:doc|unparsed-text|doc-available|"
                        r"unparsed-text-lines|unparsed-text-available)"
                        r"\s*\(\s*[\"']https?://")),
    ("build-tree", re.compile(r"\$RBKT_BIN_DIR")),
    ("nondeterministic", re.compile(
        r"\b(?:fn:)?current-(?:dateTime|date|time)\s*\(|"
        r"\b(?:fn:)?implicit-timezone\s*\(|\brandom-number-generator\s*\(|"
        r"\b(?:available-)?environment-variables?\s*\(")),
]

EXTERNAL_VAR_RE = re.compile(
    r"\bdeclare\s+variable\s+\$([\w.:-]+)(?:\s+as\s+[^;]*?)?\s+external\s*"
    r"(:=)?", re.S)
COMMENT_RE = re.compile(r"\(:.*?:\)", re.S)


class HarnessError(Exception):
    pass


# --------------------------------------------------------------------------
# Corpus selection
# --------------------------------------------------------------------------

def strip_comments(text):
    # XQuery comments nest; strip innermost first until none are left.
    prev = None
    while prev != text:
        prev = text
        text = re.sub(r"\(:(?:(?!\(:|:\)).)*:\)", " ", text, flags=re.S)
    return text


def parse_spec(path):
    """Parse the parts of an rbkt .spec file the harness uses.

    Returns a dict with 'error' (list of expected error codes),
    'vars' (list of (name, kind, value), kind 'doc' or 'string'),
    'serialization' (list of strings) and 'unsupported' (reason or None).
    """
    spec = {"error": [], "vars": [], "serialization": [], "unsupported": None}
    if not os.path.exists(path):
        return spec
    with open(path, encoding="utf-8", errors="replace") as f:
        lines = [l.strip() for l in f]
    keywords = ("Args:", "Options:", "Serialization:", "Error:", "Date:",
                "Result:", "Comparison:", "InputQuery:", "DefaultCollection:")
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith("Error:"):
            spec["error"].extend(line[len("Error:"):].split())
        elif line.startswith("Serialization:"):
            spec["serialization"].append(line[len("Serialization:"):].strip())
        elif line.startswith(("Options:", "Date:", "InputQuery:",
                              "DefaultCollection:")):
            spec["unsupported"] = "spec-" + line.split(":", 1)[0].lower()
        elif line.startswith("Args:"):
            i += 1
            while i < len(lines) and not lines[i].startswith(keywords):
                arg = lines[i]
                if arg == "-x" and i + 1 < len(lines):
                    i += 1
                    binding = lines[i]
                    if ":=" in binding:
                        name, value = binding.split(":=", 1)
                        kind = "string"
                    elif "=" in binding:
                        name, value = binding.split("=", 1)
                        kind = "doc"
                    else:
                        spec["unsupported"] = "spec-args"
                        break
                    if kind == "string" and "xs:" in value:
                        spec["unsupported"] = "spec-typed-variable"
                    spec["vars"].append((name.strip(), kind, value.strip()))
                elif arg:
                    spec["unsupported"] = "spec-args"
                i += 1
            continue
        i += 1
    return spec


def classify_query(rel, text, spec):
    """Return the skip reason for a query, or None if it is selected."""
    top = rel.split("/", 1)[0]
    if top in EXCLUDED_DIRS:
        return EXCLUDED_DIRS[top]
    if spec["unsupported"]:
        return spec["unsupported"]
    for ser in spec["serialization"]:
        if re.search(r"method=(?:json|jsoniq|json-xml-hybrid|binary)", ser):
            return "jsoniq"
    code = strip_comments(text)
    for reason, rx in CONTENT_RULES:
        if rx.search(code):
            return reason
    bound = {name.lstrip("$") for name, _, _ in spec["vars"]}
    for m in EXTERNAL_VAR_RE.finditer(code):
        name = m.group(1)
        if m.group(2) is None and name not in bound:
            return "external-variable"
    if any(name == "." for name in bound):
        return "external-context-item"
    return None


def select(rbkt, prefixes):
    """Yield one dict per .xq file under rbkt/Queries/zorba, sorted by id."""
    root = os.path.join(rbkt, "Queries", "zorba")
    found = []
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames.sort()
        for fn in sorted(filenames):
            if not fn.endswith(".xq") or fn.startswith(".difftest."):
                continue
            path = os.path.join(dirpath, fn)
            rel = os.path.relpath(path, root)[:-3].replace(os.sep, "/")
            if prefixes and not any(rel.startswith(p) for p in prefixes):
                continue
            found.append((rel, path))
    found.sort()
    for rel, path in found:
        with open(path, encoding="utf-8", errors="replace") as f:
            text = f.read()
        spec = parse_spec(path[:-3] + ".spec")
        yield {"id": rel, "path": path, "text": text, "spec": spec,
               "skip": classify_query(rel, text, spec)}


# --------------------------------------------------------------------------
# Running the processors
# --------------------------------------------------------------------------

def file_uri(path):
    return "file://" + urllib.parse.quote(os.path.abspath(path))


def substitute(value, rbkt):
    value = value.replace("$RBKT_SRC_DIR_URI", file_uri(rbkt))
    return value.replace("$RBKT_SRC_DIR", os.path.abspath(rbkt))


def prepare_query(q, rbkt):
    """Return the path of the query file to run, after the substitutions the
    rbkt test driver performs. The copy lives next to the original so that
    relative URIs resolve the same way."""
    text = q["text"]
    if "$RBKT_SRC_DIR" not in text:
        return q["path"]
    text = text.replace("$RBKT_SRC_DIR", file_uri(rbkt))
    d, fn = os.path.split(q["path"])
    path = os.path.join(d, ".difftest." + fn)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)
    return path


def has_output_declarations(text):
    return OUTPUT_NS in text or re.search(r"\boutput:[\w-]+", text) is not None


def zorba_command(zorba, qpath, q, rbkt):
    cmd = [zorba, "-f", "-q", qpath, "-r", "-x"]
    if not has_output_declarations(q["text"]):
        cmd += ["-z", "method=xml", "-z", "indent=no"]
    for name, kind, value in q["spec"]["vars"]:
        name = name.lstrip("$")
        value = substitute(value, rbkt)
        cmd += ["-e", name + ("=" if kind == "doc" else ":=") + value]
    return cmd


def saxon_command(java, classpath, qpath, q, rbkt):
    cmd = [java, "-Xss8m", "-XX:+UseSerialGC", "-XX:TieredStopAtLevel=1",
           "-Xshare:auto", "-cp", classpath, "net.sf.saxon.Query",
           "-q:" + qpath, "!omit-xml-declaration=yes"]
    if not has_output_declarations(q["text"]):
        cmd += ["!method=xml", "!indent=no"]
    for name, kind, value in q["spec"]["vars"]:
        name = name.lstrip("$")
        value = substitute(value, rbkt)
        cmd.append(("+" if kind == "doc" else "") + name + "=" + value)
    return cmd


def run_process(cmd, timeout, cwd):
    """Run a command; return dict(rc, out, err, timeout, seconds)."""
    start = time.monotonic()
    try:
        p = subprocess.Popen(cmd, stdin=subprocess.DEVNULL,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             cwd=cwd, start_new_session=True)
    except OSError as e:
        raise HarnessError("cannot run %s: %s" % (cmd[0], e))
    try:
        out, err = p.communicate(timeout=timeout)
        timed_out = False
    except subprocess.TimeoutExpired:
        try:
            os.killpg(p.pid, signal.SIGKILL)
        except OSError:
            pass
        out, err = p.communicate()
        timed_out = True
    return {"rc": p.returncode, "out": out.decode("utf-8", "replace"),
            "err": err.decode("utf-8", "replace"), "timeout": timed_out,
            "seconds": time.monotonic() - start}


STD_CODE_RE = re.compile(r"\b((?:XP|XQ|FO|SE|SR|XU|FT|XT)[A-Z]{2}\d{4})\b")
ANY_CODE_RE = re.compile(r"\b([A-Z]{4}\d{4})\b")
EQNAME_RE = re.compile(r"Q\{([^}]*)\}([\w.-]+)")
JAVA_EXC_RE = re.compile(r"Exception in thread \"[^\"]*\" (?:[\w$]+\.)*([\w$]+)")
SAXON_LEXICAL_CODE_RE = re.compile(r"^\s+([A-Za-z_][\w.-]*:[A-Za-z_][\w.-]*)\s", re.M)


def code_name(ns, local):
    if ns == ERR_NS or not ns:
        return local
    return "Q{%s}%s" % (ns, local)


def zorba_error_code(res):
    if res["rc"] < 0:
        return "CRASH:" + signal_name(-res["rc"])
    # With --print-errors-as-xml, each diagnostic is an <exception> element;
    # warnings use a namespace containing "warning".
    for ns, body in re.findall(r'<exception xmlns="([^"]*)">(.*?)</exception>',
                               res["err"], re.S):
        m = re.search(r'<code namespace="([^"]*)" local-name="([^"]*)"',
                      body)
        if m and "warn" not in ns:
            return code_name(m.group(1), m.group(2))
    m = STD_CODE_RE.search(res["err"]) or ANY_CODE_RE.search(res["err"])
    return m.group(1) if m else "UNKNOWN"


def saxon_error_code(res):
    if res["rc"] < 0:
        return "CRASH:" + signal_name(-res["rc"])
    err = strip_saxon_warnings(res["err"])
    m = JAVA_EXC_RE.search(err)
    if m:
        return "JAVA:" + m.group(1)
    m = EQNAME_RE.search(err)
    std = STD_CODE_RE.search(err)
    if m and (not std or m.start() < std.start()):
        return code_name(m.group(1), m.group(2))
    if std:
        return std.group(1)
    # Codes in other namespaces are reported as lexical QNames
    # ("  my:code  message" on the line after "Error ...").
    m = SAXON_LEXICAL_CODE_RE.search(err)
    if m:
        return m.group(1)
    m = ANY_CODE_RE.search(err)
    return m.group(1) if m else "UNKNOWN"


def strip_saxon_warnings(err):
    """Drop Saxon warning messages: a message starts with an unindented line
    and continues with indented lines; warnings start with "Warning"."""
    kept, skipping = [], False
    for line in err.splitlines():
        if line and not line[0].isspace():
            skipping = line.startswith("Warning")
        if not skipping:
            kept.append(line)
    return "\n".join(kept) + "\n"


def local_name(code):
    if code.startswith("Q{"):
        return code.split("}", 1)[1]
    return code.split(":", 1)[-1]


def codes_match(zc, sc):
    """Error codes are equal if their expanded names are equal. Saxon
    reports codes outside the standard namespace by lexical QName only, so
    a prefixed Saxon code matches a Zorba code with the same local name."""
    if "UNKNOWN" in (zc, sc):
        return False
    if zc == sc:
        return True
    if ":" in sc and not sc.startswith("Q{") and zc.startswith("Q{"):
        return local_name(zc) == local_name(sc)
    return False


def signal_name(n):
    try:
        return signal.Signals(n).name
    except ValueError:
        return str(n)


# --------------------------------------------------------------------------
# Result normalization and comparison
# --------------------------------------------------------------------------

XML_DECL_RE = re.compile(r"^\ufeff?\s*<\?xml[^?]*\?>")
WRAP = "difftest-wrapper"


def _strip(s):
    s = XML_DECL_RE.sub("", s)
    if s.endswith("\n") and not s.endswith("\n\n"):
        s = s[:-1]
    return s


def parse_fragment(s):
    """Parse a serialized result as the content of a wrapper element (with
    comments and processing instructions). None if it is not well-formed."""
    s = _strip(s)
    if "<" not in s and "&" not in s:
        return None
    parser = ET.XMLParser(target=ET.TreeBuilder(insert_comments=True,
                                                insert_pis=True))
    try:
        parser.feed("<%s>%s</%s>" % (WRAP, s, WRAP))
        return parser.close()
    except ET.ParseError:
        return None


def _escape(s, attr=False):
    s = s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    if attr:
        s = s.replace('"', "&quot;").replace("\t", "&#9;")
        s = s.replace("\n", "&#10;").replace("\r", "&#13;")
    return s.replace("\r", "&#13;")


def _serialize(e, out):
    """Prefix-independent serialization: names are written as {uri}local
    (as ElementTree reports them), attributes are sorted, namespace
    declarations are dropped; results are compared by expanded names, as
    fn:deep-equal does."""
    if e.tag is ET.Comment:
        out.append("<!--%s-->" % e.text)
    elif e.tag is ET.ProcessingInstruction:
        out.append("<?%s?>" % e.text)
    else:
        out.append("<" + e.tag)
        for k, v in sorted(e.attrib.items()):
            out.append(' %s="%s"' % (k, _escape(v, True)))
        out.append(">")
        _serialize_content(e, out)
        out.append("</%s>" % e.tag)
    out.append(_escape(e.tail or ""))


def _serialize_content(e, out):
    out.append(_escape(e.text or ""))
    for child in e:
        _serialize(child, out)


def canonical(s):
    """Canonical form of a serialized result: the XML declaration (and one
    trailing newline) is dropped and, if the output parses as an XML
    fragment, it is re-serialized by _serialize, so attribute order,
    namespace prefixes and declarations, empty-element syntax and character
    references no longer matter. Text is kept exactly, including
    whitespace."""
    root = parse_fragment(s)
    if root is None:
        return _strip(s)
    out = []
    _serialize_content(root, out)
    return "".join(out)


TOKEN_RE = re.compile(r"(<[^>]*>|\s+)")


def tokens(s):
    return [t for t in TOKEN_RE.split(s) if t and not t.isspace()]


def as_number(t):
    if t in ("INF", "-INF", "NaN"):
        return float(t)
    if re.fullmatch(r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?", t):
        return float(t)
    return None


def numbers_equal(a, b, rel):
    if math.isnan(a) and math.isnan(b):
        return True
    if a == b:
        return True
    if rel and math.isfinite(a) and math.isfinite(b):
        return abs(a - b) <= rel * max(abs(a), abs(b))
    return False


def tokens_equal(ta, tb, rel):
    if len(ta) != len(tb):
        return False
    for a, b in zip(ta, tb):
        if a == b:
            continue
        na, nb = as_number(a), as_number(b)
        if na is None or nb is None or not numbers_equal(na, nb, rel):
            return False
    return True


UNORDERED_RE = re.compile(r"\bunordered\b|\bordering\s+unordered\b")
GROUP_BY_RE = re.compile(r"\bgroup\s+by\b")
ORDER_BY_RE = re.compile(r"\border\s+by\b")


def top_level_items(s):
    """Split a serialized result into its top-level items (canonical
    serializations of nodes and whitespace-separated text tokens). None if
    it does not parse."""
    root = parse_fragment(s)
    if root is None:
        return _strip(s).split()
    items = (root.text or "").split()
    for child in root:
        tail, child.tail = child.tail, None
        out = []
        _serialize(child, out)
        items.append("".join(out))
        items.extend((tail or "").split())
    return items


def order_unspecified(text):
    """The order of the result is implementation-dependent: an unordered
    expression or ordering mode, or group by without order by (the order of
    the groups is implementation-dependent)."""
    code = strip_comments(text)
    if UNORDERED_RE.search(code):
        return "unordered"
    if GROUP_BY_RE.search(code) and not ORDER_BY_RE.search(code):
        return "group-order"
    return None


def compare(zout, sout, text):
    """Compare two serialized results. Returns (equal, note)."""
    cz, cs = canonical(zout), canonical(sout)
    if cz == cs:
        return True, ""
    tz, ts = tokens(cz), tokens(cs)
    if tz == ts:
        return False, "whitespace-only"
    if tokens_equal(tz, ts, 0.0):
        return True, "numeric-lexical"
    if tokens_equal(tz, ts, 1e-12):
        return False, "numeric-precision"
    why = order_unspecified(text)
    if why:
        if sorted(top_level_items(zout)) == sorted(top_level_items(sout)):
            return True, why
    return False, ""


def classify(zres, sres, text):
    """Return (class, note) for the two processor results."""
    if zres["timeout"] or sres["timeout"]:
        who = ("both" if zres["timeout"] and sres["timeout"] else
               "zorba" if zres["timeout"] else "saxon")
        return "timeout", who
    zerr, serr = zres["rc"] != 0, sres["rc"] != 0
    if zerr and serr:
        zc, sc = zorba_error_code(zres), saxon_error_code(sres)
        if codes_match(zc, sc):
            return "agree", "both-error"
        return "different-error-code", ""
    if zerr:
        note = "crash" if zres["rc"] < 0 else ""
        return "zorba-error-only", note
    if serr:
        return "saxon-error-only", ""
    equal, note = compare(zres["out"], sres["out"], text)
    return ("agree" if equal else "different-result"), note


# --------------------------------------------------------------------------
# run subcommand
# --------------------------------------------------------------------------

TSV_FIELDS = ["id", "class", "note", "expected", "zorba", "saxon",
              "zorba_s", "saxon_s", "query"]


def clip(s, n=300):
    s = s.replace("\\", "\\\\").replace("\t", "\\t").replace("\r", "\\r")
    s = s.replace("\n", "\\n")
    return s if len(s) <= n else s[:n] + "..."


def summary_of(res, code_fn):
    if res["timeout"]:
        return "TIMEOUT"
    if res["rc"] != 0:
        return "ERROR " + code_fn(res)
    return clip(canonical(res["out"]))


def write_raw(outdir, qid, name, res):
    base = os.path.join(outdir, "raw", qid + "." + name)
    os.makedirs(os.path.dirname(base), exist_ok=True)
    limit = 20000
    with open(base + ".out", "w", encoding="utf-8") as f:
        f.write(res["out"][:limit])
    with open(base + ".err", "w", encoding="utf-8") as f:
        f.write(res["err"][:limit])


def run_one(q, args):
    qpath = prepare_query(q, args.rbkt)
    cwd = os.path.dirname(q["path"])
    zres = run_process(zorba_command(args.zorba, qpath, q, args.rbkt),
                       args.timeout, cwd)
    sres = run_process(saxon_command(args.java, args.saxon_classpath, qpath,
                                     q, args.rbkt), args.timeout, cwd)
    cls, note = classify(zres, sres, q["text"])
    if cls != "agree":
        write_raw(args.out, q["id"], "zorba", zres)
        write_raw(args.out, q["id"], "saxon", sres)
    return {"id": q["id"], "class": cls, "note": note,
            "expected": " ".join(q["spec"]["error"]),
            "zorba": summary_of(zres, zorba_error_code),
            "saxon": summary_of(sres, saxon_error_code),
            "zorba_s": "%.2f" % zres["seconds"],
            "saxon_s": "%.2f" % sres["seconds"]}


def sanity_check(args, sides=("zorba", "saxon")):
    """Both processors must evaluate a trivial query; otherwise the harness
    is broken and nothing it reports would mean anything."""
    d = os.path.join(args.out, "sanity")
    os.makedirs(d, exist_ok=True)
    path = os.path.join(d, "sanity.xq")
    text = "<r a='1'>{1 + 1}</r>"
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)
    q = {"text": text, "spec": {"vars": []}}
    for name in sides:
        if name == "zorba":
            cmd = zorba_command(args.zorba, path, q, args.rbkt)
        else:
            cmd = saxon_command(args.java, args.saxon_classpath, path, q,
                                args.rbkt)
        res = run_process(cmd, 120, d)
        if res["rc"] != 0 or canonical(res["out"]) != '<r a="1">2</r>':
            raise HarnessError("%s sanity check failed (rc %s):\n%s\n%s" % (
                name, res["rc"], res["out"][-2000:], res["err"][-2000:]))


def write_tsv(path, rows):
    with open(path, "w", encoding="utf-8") as f:
        f.write("\t".join(TSV_FIELDS) + "\n")
        for r in rows:
            f.write("\t".join(str(r.get(k, "")) for k in TSV_FIELDS) + "\n")


def read_tsv(path):
    with open(path, encoding="utf-8") as f:
        header = f.readline().rstrip("\n").split("\t")
        for line in f:
            yield dict(zip(header, line.rstrip("\n").split("\t")))


def cmd_select(args):
    if args.set != "rbkt":
        for c in generated_cases(args):
            print("%s\t%s\t%s" % (c["id"], "solo" if c["solo"] else "batch",
                                  c["expr"].replace("\n", " ")))
        return 0
    for q in select(args.rbkt, split_prefixes(args.filter)):
        print("%s\t%s" % (q["id"], q["skip"] or "selected"))
    return 0


def split_prefixes(s):
    return [p.strip() for p in (s or "").split(",") if p.strip()]


def cmd_run(args):
    # Processes run in the query's directory, so paths must be absolute.
    args.zorba = os.path.abspath(args.zorba)
    if os.sep in args.java:
        args.java = os.path.abspath(args.java)
    if args.saxon_classpath:
        args.saxon_classpath = os.pathsep.join(
            os.path.abspath(p) for p in args.saxon_classpath.split(os.pathsep))
    elif args.set != "property":
        raise SystemExit("--saxon-classpath is required for --set %s"
                         % args.set)
    args.rbkt = os.path.abspath(args.rbkt)
    args.out = os.path.abspath(args.out)
    os.makedirs(args.out, exist_ok=True)
    if args.set != "rbkt":
        return run_generated(args)
    status = {"ok": False, "shard": args.shard, "shards": args.shards,
              "set": "rbkt"}
    try:
        sanity_check(args)
        queries = [q for i, q in enumerate(
            select(args.rbkt, split_prefixes(args.filter)))
            if i % args.shards == args.shard]
        rows = []
        todo = []
        for q in queries:
            if q["skip"]:
                rows.append({"id": q["id"], "class": "harness-skip",
                             "note": q["skip"],
                             "expected": " ".join(q["spec"]["error"])})
            else:
                todo.append(q)
        print("shard %d/%d: %d queries, %d to run" % (
            args.shard, args.shards, len(queries), len(todo)), flush=True)
        done = 0
        with concurrent.futures.ThreadPoolExecutor(args.jobs) as ex:
            futures = {ex.submit(run_one, q, args): q for q in todo}
            for fut in concurrent.futures.as_completed(futures):
                row = fut.result()
                rows.append(row)
                done += 1
                if args.verbose or row["class"] not in ("agree",):
                    print("[%d/%d] %s %s %s" % (done, len(todo), row["id"],
                                                row["class"], row["note"]),
                          flush=True)
        rows.sort(key=lambda r: r["id"])
        write_tsv(os.path.join(args.out, "results.tsv"), rows)
        status["ok"] = True
        status["counts"] = collections.Counter(r["class"] for r in rows)
    except HarnessError as e:
        status["error"] = str(e)
        print("harness error: %s" % e, file=sys.stderr)
    finally:
        with open(os.path.join(args.out, "status.json"), "w") as f:
            json.dump(status, f, indent=1, sort_keys=True)
    return 0 if status["ok"] else 2


# --------------------------------------------------------------------------
# Generated query sets (boundary, property)
# --------------------------------------------------------------------------

def generated_cases(args):
    """The cases of the generated set args.set, filtered by args.filter."""
    if args.set == "boundary":
        import boundary
        cases = boundary.cases()
    else:
        import property as prop
        cases = prop.cases(args.seed)
    prefixes = split_prefixes(args.filter)
    if prefixes:
        cases = [c for c in cases
                 if any(c["id"].startswith(p) for p in prefixes)]
    return cases


def make_batches(cases, size):
    """Group cases into batches: solo cases alone, the others in runs of
    up to size consecutive cases. The order is deterministic."""
    batches, cur = [], []
    for c in cases:
        if c["solo"]:
            batches.append([c])
            continue
        cur.append(c)
        if len(cur) >= size:
            batches.append(cur)
            cur = []
    if cur:
        batches.append(cur)
    return batches


class Stats:
    def __init__(self):
        self.lock = threading.Lock()
        self.processes = collections.Counter()
        self.splits = collections.Counter()

    def add(self, side, split):
        with self.lock:
            self.processes[side] += 1
            if split:
                self.splits[side] += 1


def batch_command(side, path, text, args):
    """The command that runs a batch query file on one processor."""
    q = {"text": text, "spec": {"vars": []}}
    if side == "saxon":
        return saxon_command(args.java, args.saxon_classpath, path, q,
                             args.rbkt)
    # The zorba command line applies its own serialization defaults (method
    # xml) over the output declarations of the query, which would escape
    # < and & in the result lines.
    return zorba_command(args.zorba, path, q, args.rbkt) + [
        "-z", "method=text"]


def run_side(side, cases, args, tag, stats):
    """Run cases on one processor; returns one result per case: the batch
    line ("<type>:<value> ...", "()" or "!<code>") or None for a timeout.
    A batch that does not complete is split until the failing cases run
    alone; a single case that fails gets the process's error code."""
    text = xqgen.batch_query([c["expr"] for c in cases])
    d = os.path.join(args.out, "queries", side)
    os.makedirs(d, exist_ok=True)
    path = os.path.join(d, tag + ".xq")
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)
    cmd = batch_command(side, path, text, args)
    single = len(cases) == 1
    res = run_process(cmd, args.timeout * (1 if single else 3), d)
    parsed = None
    if not res["timeout"] and res["rc"] == 0:
        parsed = xqgen.parse_batch_output(res["out"], len(cases))
    stats.add(side, parsed is None and not single)
    if parsed is not None or not single:
        os.remove(path)
    elif not res["timeout"]:
        # Keep the diagnostics of a case that fails on its own.
        with open(path[:-3] + ".err", "w", encoding="utf-8") as f:
            f.write(res["err"][:4000])
    if parsed is not None:
        return parsed
    if single:
        if res["timeout"]:
            return [None]
        if res["rc"] == 0:
            return ["?" + xqgen.esc_value(res["out"][:200])]
        code = (zorba_error_code if side == "zorba" else
                saxon_error_code)(res)
        return ["!" + code]
    mid = len(cases) // 2
    return (run_side(side, cases[:mid], args, tag + "a", stats) +
            run_side(side, cases[mid:], args, tag + "b", stats))


NUMERIC_TYPES = {"integer", "decimal", "float", "double", "long", "int",
                 "short", "byte", "unsignedLong", "unsignedInt",
                 "unsignedShort", "unsignedByte", "nonNegativeInteger",
                 "positiveInteger", "negativeInteger", "nonPositiveInteger"}


def _num_value(v):
    v = {"INF": "Infinity", "-INF": "-Infinity"}.get(v, v)
    try:
        return decimal.Decimal(v)
    except decimal.InvalidOperation:
        return None


def compare_values(z, s):
    """Compare two batch result lines that are not errors and not equal.
    Returns (class, note)."""
    zi, si = z.split(" "), s.split(" ")
    if len(zi) != len(si):
        return "different-result", ""
    notes = set()
    for a, b in zip(zi, si):
        if a == b:
            continue
        ta, _, va = a.partition(":")
        tb, _, vb = b.partition(":")
        if ta != tb:
            if va == vb:
                notes.add("type-only")
                continue
            return "different-result", ""
        if ta not in NUMERIC_TYPES:
            return "different-result", ""
        na, nb = _num_value(va), _num_value(vb)
        if na is None or nb is None:
            return "different-result", ""
        if na.is_nan() and nb.is_nan():
            notes.add("numeric-lexical")
        elif na == nb:
            notes.add("numeric-lexical")
        elif (na.is_finite() and nb.is_finite() and
              abs(na - nb) <= decimal.Decimal("1e-15") * max(abs(na),
                                                              abs(nb))):
            notes.add("numeric-precision")
        else:
            return "different-result", ""
    if notes <= {"numeric-lexical"}:
        return "agree", "numeric-lexical"
    if "type-only" in notes:
        return "different-result", "type-only"
    return "different-result", "numeric-precision"


def classify_case(z, s):
    """Classify the two results of a boundary case (see run_side)."""
    if z is None or s is None:
        return "timeout", ("both" if z is None and s is None else
                           "zorba" if z is None else "saxon")
    ze, se = z.startswith("!"), s.startswith("!")
    if ze and se:
        if codes_match(z[1:], s[1:]):
            return "agree", "both-error"
        return "different-error-code", ""
    if ze:
        return "zorba-error-only", "crash" if z.startswith("!CRASH") else ""
    if se:
        return "saxon-error-only", "crash" if s.startswith("!CRASH") else ""
    if z == s:
        return "agree", ""
    return compare_values(z, s)


def classify_property(z):
    if z is None:
        return "timeout", ""
    if z == "boolean:true":
        return "pass", ""
    if z.startswith("!"):
        return "error", "crash" if z.startswith("!CRASH") else ""
    return "fail", ""


def result_summary(r):
    if r is None:
        return "TIMEOUT"
    if r.startswith("!"):
        return "ERROR " + r[1:]
    return clip(r)


def run_batch(batch, index, args, stats):
    sides = ["zorba"] if args.set == "property" else ["zorba", "saxon"]
    results = {}
    seconds = {}
    for side in sides:
        start = time.monotonic()
        results[side] = run_side(side, batch, args, "b%05d" % index, stats)
        seconds[side] = time.monotonic() - start
    rows = []
    for i, c in enumerate(batch):
        z = results["zorba"][i]
        if args.set == "property":
            s = ""
            cls, note = classify_property(z)
        else:
            s = results["saxon"][i]
            cls, note = classify_case(z, s)
        row = {"id": c["id"], "class": cls, "note": note, "expected": "",
               "zorba": result_summary(z),
               "saxon": result_summary(s) if s != "" else "",
               "zorba_s": "%.2f" % (seconds["zorba"] / len(batch)),
               "saxon_s": ("%.2f" % (seconds["saxon"] / len(batch))
                           if "saxon" in seconds else ""),
               "query": clip(c["expr"], 2000)}
        rows.append(row)
    return rows


def batch_sanity_check(args, sides):
    """Every processor must run the batch format (prelude, try/catch,
    result lines); otherwise every case would be reported as an error."""
    exprs = ["1 + 1", "xs:double('INF')", "()",
             "error(xs:QName('err:FOER0000'))", "'<&amp;>'"]
    want = ["integer:2", "double:INF", "()", "!FOER0000", "string:<&>"]
    d = os.path.join(args.out, "sanity")
    os.makedirs(d, exist_ok=True)
    path = os.path.join(d, "batch.xq")
    text = xqgen.batch_query(exprs)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)
    for side in sides:
        res = run_process(batch_command(side, path, text, args), 120, d)
        got = (xqgen.parse_batch_output(res["out"], len(exprs))
               if res["rc"] == 0 else None)
        if got != want:
            raise HarnessError("%s batch sanity check failed (rc %s): got %r"
                               "\n%s\n%s" % (side, res["rc"], got,
                                              res["out"][-2000:],
                                              res["err"][-2000:]))


def run_generated(args):
    status = {"ok": False, "shard": args.shard, "shards": args.shards,
              "set": args.set}
    if args.set == "property":
        status["seed"] = args.seed
        print("property seed: %s" % args.seed, flush=True)
    stats = Stats()
    try:
        sides = ("zorba",) if args.set == "property" else ("zorba", "saxon")
        sanity_check(args, sides)
        batch_sanity_check(args, sides)
        cases = generated_cases(args)
        batches = make_batches(cases, args.batch_size)
        mine = [(i, b) for i, b in enumerate(batches)
                if i % args.shards == args.shard]
        ncases = sum(len(b) for _, b in mine)
        print("shard %d/%d: %d of %d %s cases in %d batches" % (
            args.shard, args.shards, ncases, len(cases), args.set,
            len(mine)), flush=True)
        rows = []
        done = 0
        bad = ("agree", "pass")
        with concurrent.futures.ThreadPoolExecutor(args.jobs) as ex:
            futures = [ex.submit(run_batch, b, i, args, stats)
                       for i, b in mine]
            for fut in concurrent.futures.as_completed(futures):
                batch_rows = fut.result()
                rows.extend(batch_rows)
                done += 1
                for row in batch_rows:
                    if args.verbose or row["class"] not in bad:
                        print("%s %s %s" % (row["id"], row["class"],
                                            row["note"]), flush=True)
                if done % 20 == 0:
                    print("[%d/%d batches]" % (done, len(mine)), flush=True)
        rows.sort(key=lambda r: r["id"])
        write_tsv(os.path.join(args.out, "results.tsv"), rows)
        status["ok"] = True
        status["counts"] = collections.Counter(r["class"] for r in rows)
        status["cases"] = len(rows)
        status["batches"] = len(mine)
        status["processes"] = dict(stats.processes)
        status["split_batches"] = dict(stats.splits)
    except HarnessError as e:
        status["error"] = str(e)
        print("harness error: %s" % e, file=sys.stderr)
    finally:
        with open(os.path.join(args.out, "status.json"), "w") as f:
            json.dump(status, f, indent=1, sort_keys=True)
    return 0 if status["ok"] else 2


# --------------------------------------------------------------------------
# report subcommand
# --------------------------------------------------------------------------

def table(header, rows):
    out = ["| " + " | ".join(header) + " |",
           "|" + "|".join("---" for _ in header) + "|"]
    for r in rows:
        out.append("| " + " | ".join(str(x) for x in r) + " |")
    return "\n".join(out)


def md_cell(s, n=80):
    s = s if len(s) <= n else s[:n] + "..."
    return "`%s`" % s.replace("|", "\\|").replace("`", "'") if s else ""


def cmd_report(args):
    rows, errors, statuses = [], [], []
    for d in args.shard_dirs:
        status_path = os.path.join(d, "status.json")
        if not os.path.exists(status_path):
            errors.append("%s: no status.json (shard did not finish)" % d)
            continue
        with open(status_path) as f:
            status = json.load(f)
        if not status.get("ok"):
            errors.append("%s: %s" % (d, status.get("error", "failed")))
            continue
        statuses.append(status)
        rows.extend(read_tsv(os.path.join(d, "results.tsv")))
    if args.expect_shards and len(args.shard_dirs) != args.expect_shards:
        errors.append("expected %d shard results, found %d" % (
            args.expect_shards, len(args.shard_dirs)))
    rows.sort(key=lambda r: r["id"])
    os.makedirs(args.out, exist_ok=True)
    write_tsv(os.path.join(args.out, "results.tsv"), rows)

    counts = collections.Counter(r["class"] for r in rows)
    ran = len(rows) - counts["harness-skip"]
    generated = args.set != "rbkt"
    classes = (PROPERTY_CLASSES if args.set == "property" else
               CLASSES[:-1] if generated else CLASSES)
    if args.set == "property":
        md = ["# Property tests: Zorba", ""]
        seeds = sorted({str(s.get("seed")) for s in statuses})
        md += ["Seed %s. %d cases. Reproduce a case with `difftest.py run "
               "--set property --seed %s --filter <case id> ...`."
               % (", ".join(seeds), len(rows), seeds[0] if seeds else "N"),
               ""]
    else:
        md = ["# Differential testing (%s): Zorba vs Saxon-HE" % args.set, ""]
        if args.saxon_version:
            md += ["Saxon-HE %s. " % args.saxon_version]
        if generated:
            md += ["%d generated %s cases run on both processors."
                   % (len(rows), args.set), ""]
        else:
            md += ["%d rbkt queries scanned, %d run on both processors, "
                   "%d skipped." % (len(rows), ran, counts["harness-skip"]),
                   ""]
    if generated and statuses:
        procs = collections.Counter()
        splits = collections.Counter()
        for s in statuses:
            procs.update(s.get("processes", {}))
            splits.update(s.get("split_batches", {}))
        md += ["Batches: %d; processes: %s; split batches: %s." % (
            sum(s.get("batches", 0) for s in statuses),
            ", ".join("%s %d" % kv for kv in sorted(procs.items())),
            ", ".join("%s %d" % kv for kv in sorted(splits.items())) or "0"),
            ""]
    if errors:
        md += ["**Harness errors:**", ""] + ["- " + e for e in errors] + [""]
    md += ["## Classes", "",
           table(["class", "cases" if generated else "queries", "% of run"],
                 [(c, counts[c], "-" if c == "harness-skip" else
                   "%.1f%%" % (100.0 * counts[c] / ran) if ran else "-")
                  for c in classes]), ""]

    notes = collections.Counter((r["class"], r["note"]) for r in rows
                                if r["note"] and r["class"] != "harness-skip")
    if notes:
        md += ["## Notes", "",
               table(["class", "note", "queries"],
                     [(c, n, k) for (c, n), k in notes.most_common()]), ""]

    if not generated:
        skips = collections.Counter(r["note"] for r in rows
                                    if r["class"] == "harness-skip")
        md += ["## Skip reasons", "",
               table(["reason", "queries"], skips.most_common()), ""]

    by_dir = collections.defaultdict(collections.Counter)
    for r in rows:
        by_dir[r["id"].split("/", 1)[0]][r["class"]] += 1
    label = "family" if generated else "directory"
    md += ["## By %s" % label, "",
           table([label] + classes,
                 [[d] + [by_dir[d][c] for c in classes]
                  for d in sorted(by_dir)]), ""]
    if generated:
        by_sub = collections.defaultdict(collections.Counter)
        for r in rows:
            by_sub[r["id"].rsplit("/", 1)[0]][r["class"]] += 1
        bad = [k for k in sorted(by_sub)
               if by_sub[k][classes[0]] != sum(by_sub[k].values())]
        if bad:
            md += ["## Disagreeing sub-families", "",
                   table(["family/sub"] + classes,
                         [[k] + [by_sub[k][c] for c in classes]
                          for k in bad]), ""]

    def err_code(s):
        return s[len("ERROR "):] if s.startswith("ERROR ") else s

    pairs = collections.Counter(
        (err_code(r["zorba"]), err_code(r["saxon"])) for r in rows
        if r["class"] == "different-error-code")
    if pairs:
        md += ["## Different error codes (Zorba, Saxon)", "",
               table(["zorba", "saxon", "queries"],
                     [(z, s, n) for (z, s), n in pairs.most_common(30)]), ""]
    for cls, side in (("zorba-error-only", "zorba"),
                      ("saxon-error-only", "saxon")):
        c = collections.Counter(err_code(r[side]) for r in rows
                                if r["class"] == cls)
        if c:
            md += ["## %s: error codes" % cls, "",
                   table(["code", "queries"], c.most_common(30)), ""]

    for cls in classes[1:6]:
        sel = [r for r in rows if r["class"] == cls]
        if not sel:
            continue
        if args.set == "property":
            md += ["## %s (%d)" % (cls, len(sel)), "",
                   table(["case", "note", "zorba", "query"],
                         [(r["id"], r["note"], md_cell(r["zorba"]),
                           md_cell(r.get("query", ""), 160))
                          for r in sel[:args.max_rows]]), ""]
        elif generated:
            md += ["## %s (%d)" % (cls, len(sel)), "",
                   table(["case", "note", "query", "zorba", "saxon"],
                         [(r["id"], r["note"], md_cell(r.get("query", ""), 100),
                           md_cell(r["zorba"]), md_cell(r["saxon"]))
                          for r in sel[:args.max_rows]]), ""]
        else:
            md += ["## %s (%d)" % (cls, len(sel)), "",
                   table(["query", "note", "expected", "zorba", "saxon"],
                         [(r["id"], r["note"], r["expected"],
                           md_cell(r["zorba"]), md_cell(r["saxon"]))
                          for r in sel[:args.max_rows]]), ""]
        if len(sel) > args.max_rows:
            md += ["%d more in results.tsv." % (len(sel) - args.max_rows), ""]

    text = "\n".join(md) + "\n"
    with open(os.path.join(args.out, "summary.md"), "w",
              encoding="utf-8") as f:
        f.write(text)
    if args.summary:
        # The step summary is limited to 1 MiB.
        with open(args.summary, "a", encoding="utf-8") as f:
            f.write(text[:900000])
    for e in errors:
        print("::error::%s" % e)
    print("\n".join("%s: %d" % (c, counts[c]) for c in classes))
    return 1 if errors else 0


def main(argv=None):
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    sub = ap.add_subparsers(dest="command", required=True)

    def common(p):
        p.add_argument("--set", choices=SETS, default="rbkt",
                       help="query set (default: rbkt)")
        p.add_argument("--seed", default="1",
                       help="property set: random seed (default: 1)")
        p.add_argument("--rbkt", default="test/rbkt",
                       help="rbkt source directory (default: test/rbkt)")
        p.add_argument("--filter", default="",
                       help="comma-separated query id prefixes, e.g. "
                            "'string/,numerics/'")

    p = sub.add_parser("select", help="print the corpus selection")
    common(p)
    p.set_defaults(func=cmd_select)

    p = sub.add_parser("run", help="run one shard")
    common(p)
    p.add_argument("--zorba", required=True)
    p.add_argument("--java", default="java")
    p.add_argument("--saxon-classpath", default="",
                   help="required except for --set property")
    p.add_argument("--shard", type=int, default=0)
    p.add_argument("--shards", type=int, default=1)
    p.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    p.add_argument("--timeout", type=float, default=30.0,
                   help="per-query, per-processor timeout in seconds "
                        "(a batch of generated cases gets three times this)")
    p.add_argument("--batch-size", type=int, default=100,
                   help="generated sets: cases per query (default: 100)")
    p.add_argument("--out", required=True)
    p.add_argument("--verbose", action="store_true")
    p.set_defaults(func=cmd_run)

    p = sub.add_parser("report", help="merge shard results")
    p.add_argument("shard_dirs", nargs="*")
    p.add_argument("--set", choices=SETS, default="rbkt")
    p.add_argument("--expect-shards", type=int, default=0)
    p.add_argument("--saxon-version", default="")
    p.add_argument("--summary", help="append the report to this file")
    p.add_argument("--max-rows", type=int, default=200)
    p.add_argument("--out", required=True)
    p.set_defaults(func=cmd_report)

    args = ap.parse_args(argv)
    if args.command == "run" and not (0 <= args.shard < args.shards):
        ap.error("--shard must be in [0, --shards)")
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
