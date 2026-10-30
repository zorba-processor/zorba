#!/usr/bin/env python3
"""Zorba query benchmark runner (stdlib only).

Run:      run.py run --zorba BIN --data DIR --out result.json [--md result.md]
Compare:  run.py compare A.json B.json [--md compare.md]

Every query runs once as warm-up (not recorded) and then RUNS times; each run
is a separate zorba process. Per run we record the process wall time, the
engine's own split from `zorba --timing` (compile / load / execute+serialize /
total, in ms) and the peak RSS of the process (wait4 rusage). Output goes to a
scratch file, so serialization cost is part of "execute".
A query that fails or exceeds the timeout in the warm-up run is reported and
skipped; it never fails the script.
"""
import argparse
import json
import os
import re
import statistics
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))

XMARK_SCALES = ["0.01", "0.1", "1"]
JSON_SCALES = {"10mb": 10, "100mb": 100}

TIMING_RE = re.compile(
    r"^(Engine Startup|Average Compilation|Average Execution|Average Loading|"
    r"Average Unloading|Average Total) (?:Time)\s*:\s*([\d.]+)", re.M)
TIMING_KEYS = {"Engine Startup": "startup", "Average Compilation": "compile",
               "Average Execution": "exec", "Average Loading": "load",
               "Average Unloading": "unload", "Average Total": "total"}


def data_path(data_dir, suite, scale):
    if suite == "xmark":
        return os.path.join(data_dir, "xmark-%s.xml" % scale)
    return os.path.join(data_dir, "orders-%s.json" % scale)


def build_jobs(args):
    jobs = []
    if "xmark" in args.suites:
        for scale in XMARK_SCALES:
            if args.xmark_scales and scale not in args.xmark_scales:
                continue
            for q in sorted(os.listdir(os.path.join(HERE, "xmark"))):
                if q.endswith(".xq") and (not args.only or any(o in q for o in args.only)):
                    jobs.append(("xmark", q[:-3], scale))
    if "json" in args.suites:
        for scale in JSON_SCALES:
            if args.json_scales and scale not in args.json_scales:
                continue
            for q in sorted(os.listdir(os.path.join(HERE, "json"))):
                if q.endswith(".jq") and (not args.only or any(o in q for o in args.only)):
                    jobs.append(("json", q[:-3], scale))
    return jobs


def run_once(zorba, suite, query, data_file, timeout, scratch):
    qfile = os.path.join(HERE, suite, query + (".xq" if suite == "xmark" else ".jq"))
    out_file = os.path.join(scratch, "result.out")
    if suite == "xmark":
        var = "auction=" + data_file
    else:
        var = "file:=" + data_file
    cmd = [zorba, "--omit-xml-declaration", "--timing", "-o", out_file,
           "-e", var, "-f", "-q", qfile]
    so_path = os.path.join(scratch, "stdout")
    se_path = os.path.join(scratch, "stderr")
    res = {"status": "ok"}
    with open(so_path, "wb") as so, open(se_path, "wb") as se:
        t0 = time.monotonic()
        proc = subprocess.Popen(cmd, stdout=so, stderr=se)
        rusage = None
        status = None
        while True:
            pid, st, ru = os.wait4(proc.pid, os.WNOHANG)
            if pid:
                status, rusage = st, ru
                break
            if time.monotonic() - t0 > timeout:
                proc.kill()
                _, status, rusage = os.wait4(proc.pid, 0)
                res["status"] = "timeout"
                break
            time.sleep(0.02)
        res["wall"] = time.monotonic() - t0
    proc.returncode = 0  # reaped by wait4; avoid Popen's own wait
    if rusage is not None:
        rss = rusage.ru_maxrss
        res["rss_mb"] = rss / (1024.0 * 1024.0) if sys.platform == "darwin" else rss / 1024.0
    if res["status"] == "ok":
        code = os.waitstatus_to_exitcode(status)
        if code != 0:
            res["status"] = "error"
            with open(se_path, "r", errors="replace") as f:
                err = f.read().strip()
            with open(so_path, "r", errors="replace") as f:
                err = (err + " " + f.read().strip()).strip()
            res["error"] = "exit %d: %s" % (code, err[:300].replace("\n", " "))
        else:
            with open(so_path, "r", errors="replace") as f:
                for k, v in TIMING_RE.findall(f.read()):
                    res[TIMING_KEYS[k]] = float(v) / 1000.0
            try:
                res["out_bytes"] = os.path.getsize(out_file)
            except OSError:
                pass
    try:
        os.remove(out_file)
    except OSError:
        pass
    return res


def summarize(runs):
    out = {}
    for key in ("wall", "compile", "load", "exec", "total"):
        vals = [r[key] for r in runs if key in r]
        if vals:
            out[key + "_median"] = statistics.median(vals)
            out[key + "_min"] = min(vals)
    rss = [r["rss_mb"] for r in runs if "rss_mb" in r]
    if rss:
        out["rss_mb"] = max(rss)
    sizes = {r.get("out_bytes") for r in runs}
    if len(sizes) == 1:
        out["out_bytes"] = sizes.pop()
    return out


def cmd_run(args):
    results = []
    scratch = tempfile.mkdtemp(prefix="zbench-")
    for suite, query, scale in build_jobs(args):
        data_file = data_path(args.data, suite, scale)
        entry = {"suite": suite, "query": query, "scale": scale}
        if not os.path.exists(data_file):
            entry.update(status="nodata")
            results.append(entry)
            continue
        first = run_once(args.zorba, suite, query, data_file, args.timeout, scratch)
        if first["status"] != "ok":
            entry.update(status=first["status"], error=first.get("error"),
                         rss_mb=first.get("rss_mb"))
            print("%-6s %-20s %-6s %s %s" % (suite, query, scale, first["status"],
                  first.get("error", "")), flush=True)
            results.append(entry)
            continue
        n = args.runs
        if first["wall"] > args.slow:  # keep the total job time bounded
            n = min(n, 3)
        runs = []
        for _ in range(n):
            r = run_once(args.zorba, suite, query, data_file, args.timeout, scratch)
            if r["status"] != "ok":
                entry.update(status=r["status"], error=r.get("error"))
                break
            runs.append(r)
        if "status" not in entry:
            entry["status"] = "ok"
            entry["runs"] = len(runs)
            entry.update(summarize(runs))
        print("%-6s %-20s %-6s %s wall=%.3fs rss=%.0fMB" % (
            suite, query, scale, entry["status"],
            entry.get("wall_median", -1), entry.get("rss_mb", -1)), flush=True)
        results.append(entry)
    doc = {"label": args.label, "zorba": args.zorba, "runs": args.runs,
           "timeout": args.timeout, "results": results}
    with open(args.out, "w") as f:
        json.dump(doc, f, indent=1)
    if args.md:
        with open(args.md, "w") as f:
            f.write(render_run(doc))
    return 0


def scales_of(suite):
    return XMARK_SCALES if suite == "xmark" else list(JSON_SCALES)


def index(doc):
    return {(r["suite"], r["query"], r["scale"]): r for r in doc["results"]}


def cell(r, key="wall_median"):
    if r is None or r["status"] == "nodata":
        return "n/a"
    if r["status"] == "timeout":
        return "TIMEOUT"
    if r["status"] == "error":
        return "ERROR"
    return "%.3f" % r[key] if key in r else "n/a"


def render_run(doc):
    idx = index(doc)
    lines = ["## Zorba benchmark: %s" % doc.get("label", ""), "",
             "Median wall time in seconds over %d runs after 1 warm-up (min in "
             "parentheses); timeout %ds. Each run is a separate process; time "
             "includes engine start-up, document load, compile, execute and "
             "serialize." % (doc["runs"], doc["timeout"]), ""]
    for suite in ("xmark", "json"):
        qs = sorted({k[1] for k in idx if k[0] == suite})
        if not qs:
            continue
        sc = scales_of(suite)
        lines.append("### %s: wall time (s)" % suite)
        lines.append("")
        lines.append("| query | " + " | ".join(sc) + " | " + "x(%s/%s)" % (sc[-1], sc[-2]) + " |")
        lines.append("|---|" + "---:|" * (len(sc) + 1))
        for q in qs:
            row = []
            for s in sc:
                r = idx.get((suite, q, s))
                c = cell(r)
                if r and r["status"] == "ok":
                    c += " (%.3f)" % r["wall_min"]
                row.append(c)
            a, b = idx.get((suite, q, sc[-2])), idx.get((suite, q, sc[-1]))
            ratio = "-"
            if a and b and a["status"] == "ok" and b["status"] == "ok" and a["wall_median"] > 0:
                ratio = "%.1f" % (b["wall_median"] / a["wall_median"])
            lines.append("| %s | %s | %s |" % (q, " | ".join(row), ratio))
        lines += ["", "Data ratio between the last two scales is 10x (xmark) / 10x (json); "
                  "a ratio well above 10 means super-linear growth.", ""]
        lines.append("### %s: engine split at the largest scale (s)" % suite)
        lines.append("")
        lines.append("| query | compile | load | execute+serialize | peak RSS (MB) |")
        lines.append("|---|---:|---:|---:|---:|")
        for q in qs:
            r = idx.get((suite, q, sc[-1]))
            if r and r["status"] == "ok":
                lines.append("| %s | %s | %s | %s | %.0f |" % (
                    q, cell(r, "compile_median"), cell(r, "load_median"),
                    cell(r, "exec_median"), r.get("rss_mb", 0)))
            else:
                lines.append("| %s | %s | | | %s |" % (
                    q, cell(r), "%.0f" % r["rss_mb"] if r and r.get("rss_mb") else ""))
        lines.append("")
        lines.append("### %s: peak RSS (MB)" % suite)
        lines.append("")
        lines.append("| query | " + " | ".join(sc) + " |")
        lines.append("|---|" + "---:|" * len(sc))
        for q in qs:
            row = []
            for s in sc:
                r = idx.get((suite, q, s))
                row.append("%.0f" % r["rss_mb"] if r and r.get("rss_mb") else "-")
            lines.append("| %s | %s |" % (q, " | ".join(row)))
        lines.append("")
    problems = [r for r in doc["results"] if r["status"] not in ("ok", "nodata")]
    if problems:
        lines.append("### Failures and timeouts")
        lines.append("")
        for r in problems:
            lines.append("- %s/%s @ %s: %s %s" % (r["suite"], r["query"], r["scale"],
                         r["status"], r.get("error") or ""))
        lines.append("")
    return "\n".join(lines)


def cmd_compare(args):
    with open(args.a) as f:
        a = json.load(f)
    with open(args.b) as f:
        b = json.load(f)
    ia, ib = index(a), index(b)
    lines = ["## Zorba benchmark comparison", "",
             "A = `%s`, B = `%s`. Ratio = B/A of the median wall time "
             "(below 1 means B is faster). Differences under ~10%% on runs "
             "shorter than 1 s are within runner noise." % (a["label"], b["label"]), ""]
    for suite in ("xmark", "json"):
        keys = sorted(k for k in set(ia) | set(ib) if k[0] == suite)
        if not keys:
            continue
        sc = scales_of(suite)
        qs = sorted({k[1] for k in keys})
        lines.append("### %s: B/A wall-time ratio" % suite)
        lines.append("")
        lines.append("| query | " + " | ".join(sc) + " |")
        lines.append("|---|" + "---:|" * len(sc))
        for q in qs:
            row = []
            for s in sc:
                ra, rb = ia.get((suite, q, s)), ib.get((suite, q, s))
                if ra and rb and ra["status"] == "ok" and rb["status"] == "ok":
                    row.append("%.2f" % (rb["wall_median"] / ra["wall_median"]))
                else:
                    row.append("A:%s B:%s" % (cell(ra), cell(rb)))
            lines.append("| %s | %s |" % (q, " | ".join(row)))
        lines.append("")
        lines.append("### %s: B/A peak RSS ratio" % suite)
        lines.append("")
        lines.append("| query | " + " | ".join(sc) + " |")
        lines.append("|---|" + "---:|" * len(sc))
        for q in qs:
            row = []
            for s in sc:
                ra, rb = ia.get((suite, q, s)), ib.get((suite, q, s))
                if ra and rb and ra.get("rss_mb") and rb.get("rss_mb"):
                    row.append("%.2f" % (rb["rss_mb"] / ra["rss_mb"]))
                else:
                    row.append("-")
            lines.append("| %s | %s |" % (q, " | ".join(row)))
        lines.append("")
    # output-size differences flag behavior changes
    diff = [k for k in ia if k in ib and ia[k].get("out_bytes") is not None
            and ia[k].get("out_bytes") != ib[k].get("out_bytes")]
    if diff:
        lines.append("### Output size differs between A and B")
        lines.append("")
        for k in sorted(diff):
            lines.append("- %s/%s @ %s: %s vs %s bytes" % (k + (ia[k]["out_bytes"], ib[k]["out_bytes"])))
        lines.append("")
    text = "\n".join(lines)
    if args.md:
        with open(args.md, "w") as f:
            f.write(text)
    else:
        print(text)
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run")
    r.add_argument("--zorba", required=True)
    r.add_argument("--data", required=True)
    r.add_argument("--out", required=True)
    r.add_argument("--md")
    r.add_argument("--label", default="")
    r.add_argument("--suites", default="xmark,json")
    r.add_argument("--runs", type=int, default=5)
    r.add_argument("--timeout", type=int, default=300)
    r.add_argument("--slow", type=float, default=30.0,
                   help="warm-up wall time (s) above which only 3 runs are done")
    r.add_argument("--only", help="comma-separated substrings of query names")
    r.add_argument("--xmark-scales", help="comma-separated subset of " + ",".join(XMARK_SCALES))
    r.add_argument("--json-scales", help="comma-separated subset of " + ",".join(JSON_SCALES))
    c = sub.add_parser("compare")
    c.add_argument("a")
    c.add_argument("b")
    c.add_argument("--md")
    args = ap.parse_args()
    if args.cmd == "run":
        args.suites = args.suites.split(",")
        args.only = args.only.split(",") if args.only else None
        args.xmark_scales = args.xmark_scales.split(",") if args.xmark_scales else None
        args.json_scales = args.json_scales.split(",") if args.json_scales else None
        return cmd_run(args)
    return cmd_compare(args)


if __name__ == "__main__":
    sys.exit(main())
