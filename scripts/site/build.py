#!/usr/bin/env python3
"""Build the documentation site.

Usage: build.py <site-dir> <output-dir> [--strict]

Copies <site-dir> to <output-dir> and, in every top-level .html page, replaces

  <!-- example: section/name -->   with the highlighted query
                                   section/name.{xq,jq,cpp} and its verified
                                   output section/name.out
  <!-- source: path/to/file -->    with the highlighted contents of a file
                                   under <site-dir>/examples (data files,
                                   library modules)

With --strict, a marker whose files are missing is an error; otherwise a
placeholder is rendered. Python standard library only.
"""
import html
import re
import shutil
import sys
from pathlib import Path

KEYWORDS = set("""
for let where order by group count return in at as allowing empty stable
ascending descending collation tumbling sliding window start end only when
previous next if then else switch case default typeswitch some every satisfies
and or not div idiv mod eq ne lt le gt ge is to union intersect except
instance of treat cast castable try catch declare function variable external
module namespace import schema option context item copy modify insert delete
replace value rename node nodes into before after first last with element
attribute text comment document processing-instruction contains using stemming
wildcards ftand ftor ftnot words window any all word phrase while exit
returning break continue collection index integrity constraint on unique key
check foreach json append position select jsoniq xquery version encoding
object array null true false boolean string integer decimal double
""".split())

CPP_KEYWORDS = set("""
include using namespace int void return try catch const auto while for if else
std char bool true false
""".split())

LANG_NAMES = {".xq": "XQuery", ".xqm": "XQuery module", ".jq": "JSONiq",
              ".cpp": "C++", ".xml": "XML", ".json": "JSON"}

TOKEN_RE = {
    "query": re.compile(r"""
        (?P<comment>\(:.*?:\))
      | (?P<string>"(?:[^"]|"")*"|'(?:[^']|'')*')
      | (?P<tag></?[A-Za-z_][\w.:-]*|/?>)
      | (?P<var>\$\$|\$[A-Za-z_][\w:-]*)
      | (?P<number>\b\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b)
      | (?P<func>\b[A-Za-z_][\w-]*(?::[A-Za-z_][\w-]*)?(?=\s*[(#]))
      | (?P<word>\b[A-Za-z_][\w-]*\b)
    """, re.S | re.X),
    "cpp": re.compile(r"""
        (?P<comment>//[^\n]*|/\*.*?\*/)
      | (?P<string>"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*')
      | (?P<pre>\#\w+)
      | (?P<number>\b\d+\b)
      | (?P<func>\b[A-Za-z_]\w*(?=\())
      | (?P<word>\b[A-Za-z_]\w*\b)
    """, re.S | re.X),
    "data": re.compile(r"""
        (?P<comment><!--.*?-->)
      | (?P<string>"(?:\\.|[^"\\])*")
      | (?P<tag></?[A-Za-z_][\w.:-]*|/?>)
      | (?P<number>-?\b\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b)
      | (?P<word>\b(?:true|false|null)\b)
    """, re.S | re.X),
}

CLASS = {"comment": "c", "string": "s", "tag": "t", "var": "v", "number": "n",
         "func": "f", "word": "k", "pre": "k"}


def highlight(text: str, mode: str) -> str:
    out = []
    pos = 0
    keywords = CPP_KEYWORDS if mode == "cpp" else KEYWORDS
    for m in TOKEN_RE[mode].finditer(text):
        kind = m.lastgroup
        token = m.group(0)
        if kind == "word" and mode != "data" and token not in keywords:
            continue
        out.append(html.escape(text[pos:m.start()], quote=False))
        out.append(f'<span class="tok-{CLASS[kind]}">'
                   f'{html.escape(token, quote=False)}</span>')
        pos = m.end()
    out.append(html.escape(text[pos:], quote=False))
    return "".join(out)


def mode_for(suffix: str) -> str:
    if suffix == ".cpp":
        return "cpp"
    if suffix in (".xml", ".json", ".out"):
        return "data"
    return "query"


class Builder:
    def __init__(self, site: Path, strict: bool):
        self.site = site
        self.examples = site / "examples"
        self.strict = strict
        self.problems = []

    def missing(self, what: str) -> str:
        self.problems.append(what)
        return (f'<div class="example-missing">Missing: '
                f'{html.escape(what)}</div>')

    def example(self, name: str) -> str:
        src = next((self.examples / f"{name}{ext}"
                    for ext in (".xq", ".jq", ".cpp")
                    if (self.examples / f"{name}{ext}").exists()), None)
        if src is None:
            return self.missing(f"example {name}")
        code = src.read_text(encoding="utf-8").rstrip("\n")
        out_file = src.with_suffix(".out")
        args_file = src.with_suffix(".args")
        rel = src.relative_to(self.examples).as_posix()
        if src.suffix == ".cpp":
            command = f"g++ -std=c++23 {src.name} -lzorba_simplestore && ./a.out"
        else:
            extra = (" " + args_file.read_text().strip()) if args_file.exists() else ""
            command = f"zorba -i -r{extra} -f -q {src.name}"
        if out_file.exists():
            output = out_file.read_text(encoding="utf-8").rstrip("\n")
            result = (f'<pre class="output"><code>'
                      f'{highlight(output, "data")}</code></pre>')
        else:
            result = self.missing(f"expected output {rel}")
        lang = LANG_NAMES[src.suffix]
        return (
            f'<figure class="example" id="ex-{name.replace("/", "-")}">'
            f'<figcaption><span class="ex-lang">{lang}</span>'
            f'<a class="ex-file" href="examples/{html.escape(rel)}">'
            f'{html.escape(rel)}</a></figcaption>'
            f'<pre class="code"><code>{highlight(code, mode_for(src.suffix))}'
            f'</code></pre>'
            f'<div class="result"><div class="result-head">'
            f'<span>Result</span><code class="cmd">{html.escape(command)}</code>'
            f'</div>{result}</div></figure>')

    def source(self, rel: str) -> str:
        path = self.examples / rel
        if not path.exists():
            return self.missing(f"source {rel}")
        text = path.read_text(encoding="utf-8").rstrip("\n")
        lang = LANG_NAMES.get(path.suffix, path.suffix)
        return (
            f'<figure class="example source">'
            f'<figcaption><span class="ex-lang">{lang}</span>'
            f'<a class="ex-file" href="examples/{html.escape(rel)}">'
            f'{html.escape(rel)}</a></figcaption>'
            f'<pre class="code"><code>{highlight(text, mode_for(path.suffix))}'
            f'</code></pre></figure>')

    def page(self, text: str) -> str:
        text = re.sub(r"<!--\s*example:\s*(\S+?)\s*-->",
                      lambda m: self.example(m.group(1)), text)
        text = re.sub(r"<!--\s*source:\s*(\S+?)\s*-->",
                      lambda m: self.source(m.group(1)), text)
        return text


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    site, out = Path(args[0]), Path(args[1])
    builder = Builder(site, "--strict" in sys.argv)
    if out.exists():
        shutil.rmtree(out)
    shutil.copytree(site, out,
                    ignore=shutil.ignore_patterns("*.out", "*.args", ".*"))
    for page in sorted(site.glob("*.html")):
        (out / page.name).write_text(
            builder.page(page.read_text(encoding="utf-8")), encoding="utf-8")
        print(f"built {page.name}")
    (out / ".nojekyll").write_text("")
    for p in builder.problems:
        print(f"warning: {p}")
    if builder.strict and builder.problems:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
