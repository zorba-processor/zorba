#!/usr/bin/env python3
"""Agent benchmark harness: the same model answers the same tasks over the
same snapshot directory with two tool configurations.

  A  the zorba-mcp tools only (list_documents, run_query, and the predefined
     query tools of --queries-dir, if any), over MCP/stdio
  B  a bash tool only, in a container with the snapshot mounted read-only,
     no network and CPU/memory/time limits (python3, jq, xmllint, grep, awk)

Both configurations get the same system prompt, the same per-task prompt,
the same turn and token budget and the same submit_answer tool; only the
data tools (and their descriptions) differ. Every run is graded against
tasks/ground_truth.json.

Output (--out): runs.jsonl (one line per run, with the tool-call log),
summary.json and summary.md (per task and per configuration: accuracy, mean
tool calls, turns, tokens, cost estimate, wall time, repeatability).

--dry-run replaces the model with a scripted fake that exercises the whole
loop without an API key: in A it runs the task's ground-truth JSONiq query
(queries/<task>.jq) through zorba-mcp's run_query tool, in B it runs the
Python reference (reference.py) through the bash tool inside the sandbox,
and submits the result. Every dry run must be graded correct.
"""

import argparse
import collections
import datetime
import json
import os
import re
import shlex
import subprocess
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import grading  # noqa: E402

DEFAULT_MODEL = "claude-sonnet-5-5"
DEFAULT_SNAPSHOT = os.path.join(HERE, "data", "snapshot")
DEFAULT_IMAGE = "zorba-agent-sandbox:latest"

# USD per million tokens: input, output, cache write (5 min), cache read.
PRICES = {
    "claude-sonnet-5-5": (2.00, 10.00, 2.50, 0.20),
    "claude-opus-5-5": (4.00, 20.00, 5.00, 0.20),
    "claude-haiku-5-5": (0.10, 0.50, 0.125, 0.01),
    "claude-sonnet-5": (2.00, 10.00, 2.50, 0.20),
    "claude-opus-5": (5.00, 25.00, 6.25, 0.50),
}

SYSTEM_PROMPT = """\
You are a situational-awareness analyst. You answer questions about a frozen \
snapshot of public geospatial feeds (earthquakes, satellites, disaster \
alerts, weather, active fires and a reference web page) stored as files in \
one directory tree, in several formats (JSON, CSV, XML, HTML).

Rules:
- Answer only from the files, using your tools to read and compute. Do not \
use prior knowledge about current events.
- The files can disagree with each other, and some can be damaged; report \
what the files say.
- When you have the answer, call the submit_answer tool once, with the \
answer in the JSON shape given with the question. Do not call it before you \
are done."""

SUBMIT_DESCRIPTION = (
    "Submits the final answer to the question. Call it exactly once, when "
    "you are done; the answer must follow the JSON schema given with the "
    "question.")

BASH_DESCRIPTION = (
    "Runs a bash command in a sandbox and returns its standard output and "
    "standard error. The snapshot directory is /data (read-only; it is the "
    "working directory); /tmp is writable. Available: python3 (standard "
    "library only), jq, xmllint, grep, awk, sed, find and the coreutils. No "
    "network. Each command is limited to {timeout} seconds; output longer "
    "than {max_output} bytes is cut.")


def now_iso():
    return datetime.datetime.now(datetime.timezone.utc).strftime(
        "%Y-%m-%dT%H:%M:%SZ")


def cap_output(text, limit):
    data = text.encode("utf-8", errors="replace")
    if len(data) <= limit:
        return text
    cut = data[:limit].decode("utf-8", errors="ignore")
    return (cut + f"\n[output truncated after {limit} of {len(data)} bytes; "
            "narrow the command to see the rest]")


# --- configuration A: MCP client over stdio ----------------------------------

class MCPClient:
    """A minimal MCP client (JSON-RPC 2.0, one message per line on stdio)."""

    PROTOCOL = "2025-11-25"

    def __init__(self, command, log_path):
        self.log = open(log_path, "ab")
        self.proc = subprocess.Popen(
            command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=self.log)
        self.next_id = 0
        init = self.request("initialize", {
            "protocolVersion": self.PROTOCOL, "capabilities": {},
            "clientInfo": {"name": "zorba-agent-benchmark", "version": "1"}})
        self.server_info = init.get("serverInfo", {})
        self.notify("notifications/initialized")
        self.tools = self.request("tools/list", {}).get("tools", [])

    def _send(self, msg):
        self.proc.stdin.write((json.dumps(msg) + "\n").encode("utf-8"))
        self.proc.stdin.flush()

    def notify(self, method, params=None):
        msg = {"jsonrpc": "2.0", "method": method}
        if params is not None:
            msg["params"] = params
        self._send(msg)

    def request(self, method, params):
        self.next_id += 1
        rid = self.next_id
        self._send({"jsonrpc": "2.0", "id": rid, "method": method,
                    "params": params})
        while True:
            line = self.proc.stdout.readline()
            if not line:
                raise RuntimeError(f"MCP server exited during {method} "
                                   f"(status {self.proc.poll()})")
            msg = json.loads(line)
            if msg.get("id") != rid:
                continue                    # a notification or a stale reply
            if "error" in msg:
                raise RuntimeError(f"MCP {method}: {msg['error']}")
            return msg["result"]

    def anthropic_tools(self):
        return [{"name": t["name"], "description": t.get("description", ""),
                 "input_schema": t.get("inputSchema", {"type": "object"})}
                for t in self.tools]

    def call(self, name, args):
        result = self.request("tools/call", {"name": name,
                                             "arguments": args})
        text = "\n".join(c.get("text", "") for c in result.get("content", [])
                         if c.get("type") == "text")
        return text, bool(result.get("isError"))

    def close(self):
        try:
            self.proc.stdin.close()
            self.proc.wait(timeout=10)
        except Exception:  # noqa: BLE001 - best effort shutdown
            self.proc.kill()
        self.log.close()


class ConfigA:
    name = "A"

    def __init__(self, args, log_path):
        cmd = [part.format(snapshot=args.snapshot,
                           queries=args.queries_dir or "")
               for part in shlex.split(args.mcp_command)]
        if args.queries_dir:
            cmd += ["--queries", args.queries_dir]
        cmd += ["--timeout", str(args.tool_timeout),
                "--max-output", str(args.max_output)]
        self.client = MCPClient(cmd, log_path)
        self.tools = self.client.anthropic_tools()
        self.tool_names = {t["name"] for t in self.tools}

    def call(self, name, args):
        if name not in self.tool_names:
            return f"unknown tool {name}", True
        return self.client.call(name, args)

    def close(self):
        self.client.close()


# --- configuration B: bash in a container ------------------------------------

class ConfigB:
    name = "B"

    def __init__(self, args, log_path):
        self.timeout = args.tool_timeout
        self.max_output = args.max_output
        snapshot = os.path.abspath(args.snapshot)
        run = ["docker", "run", "-d", "--rm", "--network", "none",
               "--read-only", "--tmpfs", "/tmp:rw,size=64m",
               "--cpus", "1", "--memory", "1g", "--pids-limit", "256",
               "--security-opt", "no-new-privileges", "--cap-drop", "ALL",
               "-v", f"{snapshot}:/data:ro", "-w", "/data",
               args.sandbox_image, "sleep", "infinity"]
        self.cid = subprocess.run(run, check=True, capture_output=True,
                                  text=True).stdout.strip()
        self.tools = [{
            "name": "bash",
            "description": BASH_DESCRIPTION.format(
                timeout=self.timeout, max_output=self.max_output),
            "input_schema": {
                "type": "object",
                "properties": {"command": {
                    "type": "string",
                    "description": "The bash command to run."}},
                "required": ["command"],
                "additionalProperties": False,
            },
        }]

    def call(self, name, args):
        if name != "bash" or not isinstance(args.get("command"), str):
            return "the bash tool requires a string argument \"command\"", True
        try:
            proc = subprocess.run(
                ["docker", "exec", "-i", self.cid, "timeout", "-s", "KILL",
                 str(self.timeout), "bash", "-c", args["command"]],
                capture_output=True, timeout=self.timeout + 30,
                stdin=subprocess.DEVNULL)
        except subprocess.TimeoutExpired:
            return f"[command timed out after {self.timeout} s]", True
        out = (proc.stdout + proc.stderr).decode("utf-8", errors="replace")
        if proc.returncode == 137:
            out += f"\n[command killed: time limit of {self.timeout} s]"
        elif proc.returncode != 0:
            out += f"\n[exit status {proc.returncode}]"
        return cap_output(out, self.max_output), proc.returncode != 0

    def close(self):
        subprocess.run(["docker", "rm", "-f", self.cid],
                       capture_output=True)


# --- models -----------------------------------------------------------------

class Block(dict):
    """A content block of the fake model (attribute access like the SDK)."""
    __getattr__ = dict.get


class FakeResponse:
    def __init__(self, blocks, stop_reason, in_tokens, out_tokens):
        self.content = [Block(b) for b in blocks]
        self.stop_reason = stop_reason
        self.stop_details = None
        self.usage = Block(input_tokens=in_tokens, output_tokens=out_tokens,
                           cache_creation_input_tokens=0,
                           cache_read_input_tokens=0)


class FakeModel:
    """Scripted stand-in for the model (--dry-run). The script depends on
    the configuration, recognized by the tools it is given."""

    def __init__(self, task_id):
        self.task_id = task_id
        self.calls = 0

    def create(self, system, tools, messages):
        self.calls += 1
        names = {t["name"] for t in tools}
        step = sum(1 for m in messages if m["role"] == "assistant")
        last = messages[-1]["content"]
        last_result = ""
        if isinstance(last, list):
            for b in last:
                if b.get("type") == "tool_result":
                    last_result = b.get("content", "")
        size = len(json.dumps(messages, default=str)) // 4
        if "run_query" in names:
            script = [
                ("list_documents", {}),
                ("run_query", {"query": open(os.path.join(
                    HERE, "queries", self.task_id + ".jq")).read()}),
            ]
        else:
            # reference.py, run from stdin: no __file__, no main().
            ref = ("__file__ = '/tmp/reference.py'\n" +
                   open(os.path.join(HERE, "reference.py")).read().replace(
                       'if __name__ == "__main__":', "if False:"))
            script = [
                ("bash", {"command": "find . -type f | sort | head -50"}),
                ("bash", {"command":
                          "python3 - <<'PYEOF'\n" + ref + "\nimport json\n"
                          f"print(json.dumps(compute('/data', "
                          f"['{self.task_id}'])['{self.task_id}']))\n"
                          "PYEOF"}),
            ]
        if step < len(script):
            name, args = script[step]
            block = {"type": "tool_use", "id": f"toolu_fake_{step}",
                     "name": name, "input": args}
            return FakeResponse([{"type": "text", "text": "Looking."}, block],
                                "tool_use", size, 50)
        lines = [ln for ln in last_result.splitlines() if ln.strip()]
        try:
            answer = json.loads(lines[-1])
        except (IndexError, ValueError):
            answer = {"unparsable_tool_output": last_result[:200]}
        block = {"type": "tool_use", "id": f"toolu_fake_{step}",
                 "name": "submit_answer", "input": {"answer": answer}}
        return FakeResponse([block], "tool_use", size, 50)


class ClaudeModel:
    def __init__(self, model, max_tokens, effort):
        import anthropic  # only needed for real runs
        self.anthropic = anthropic
        self.client = anthropic.Anthropic(max_retries=6)
        self.model = model
        self.max_tokens = max_tokens
        self.effort = effort

    def create(self, system, tools, messages):
        kwargs = dict(model=self.model, max_tokens=self.max_tokens,
                      system=system, tools=tools, messages=messages,
                      cache_control={"type": "ephemeral"})
        if self.effort:
            kwargs["output_config"] = {"effort": self.effort}
        return self.client.messages.create(**kwargs)


# --- one run -----------------------------------------------------------------

def block_to_dict(b):
    if isinstance(b, dict):
        return dict(b)
    return b.to_dict()


def extract_json_answer(text):
    """Fallback when the model ends without submit_answer: the last JSON
    object in its text (in a ```json block or bare)."""
    for m in reversed(re.findall(r"```(?:json)?\s*(\{.*?\})\s*```", text,
                                 re.S)):
        try:
            return json.loads(m)
        except ValueError:
            pass
    return None


def run_task(task, config, model, args):
    submit = {"name": "submit_answer", "description": SUBMIT_DESCRIPTION,
              "input_schema": {"type": "object",
                               "properties": {"answer": task["answer_schema"]},
                               "required": ["answer"]}}
    tools = config.tools + [submit]
    prompt = (task["question"] + "\n\nAnswer JSON schema (the \"answer\" "
              "argument of submit_answer):\n" +
              json.dumps(task["answer_schema"]))
    messages = [{"role": "user", "content": prompt}]
    usage = collections.Counter()
    log, answer, stop, text = [], None, "turn_limit", ""
    turns = tool_calls = 0
    start = time.monotonic()
    while turns < args.max_turns:
        turns += 1
        try:
            resp = model.create(SYSTEM_PROMPT, tools, messages)
        except Exception as e:  # noqa: BLE001 - recorded, run continues
            stop = f"api_error: {type(e).__name__}: {str(e)[:300]}"
            break
        for k in ("input_tokens", "output_tokens",
                  "cache_creation_input_tokens", "cache_read_input_tokens"):
            usage[k] += getattr(resp.usage, k, 0) or 0
        messages.append({"role": "assistant", "content": resp.content})
        text += "".join(b.text or "" for b in resp.content
                        if b.type == "text")
        if resp.stop_reason == "refusal":
            stop = "refusal"
            break
        if resp.stop_reason == "max_tokens":
            stop = "max_tokens"
            break
        uses = [b for b in resp.content if b.type == "tool_use"]
        if not uses:
            stop = "no_answer"
            answer = extract_json_answer(text)
            if answer is not None:
                stop = "answer_in_text"
            break
        results, done = [], False
        for b in uses:
            if b.name == "submit_answer":
                inp = b.input if isinstance(b.input, dict) else {}
                answer = inp.get("answer")
                done = True
                results.append({"type": "tool_result", "tool_use_id": b.id,
                                "content": "Answer received."})
                continue
            tool_calls += 1
            t0 = time.monotonic()
            try:
                out, is_error = config.call(b.name, dict(b.input or {}))
            except Exception as e:  # noqa: BLE001 - reported to the model
                out, is_error = f"tool failure: {e}", True
            log.append({"tool": b.name, "input": b.input,
                        "output": out[:4000], "output_bytes": len(out),
                        "is_error": is_error,
                        "seconds": round(time.monotonic() - t0, 3)})
            results.append({"type": "tool_result", "tool_use_id": b.id,
                            "content": out or "(no output)",
                            "is_error": is_error})
        messages.append({"role": "user", "content": results})
        if done:
            stop = "submitted"
            break
    wall = time.monotonic() - start
    return {
        "answer": answer, "stop": stop, "turns": turns,
        "tool_calls": tool_calls, "usage": dict(usage),
        "wall_seconds": round(wall, 2), "tool_log": log,
        "final_text": text[-2000:],
    }


def cost_usd(model, usage):
    p = PRICES.get(model)
    if not p:
        return None
    return round((usage.get("input_tokens", 0) * p[0] +
                  usage.get("output_tokens", 0) * p[1] +
                  usage.get("cache_creation_input_tokens", 0) * p[2] +
                  usage.get("cache_read_input_tokens", 0) * p[3]) / 1e6, 6)


# --- summary -----------------------------------------------------------------

def canonical(x):
    return json.dumps(x, sort_keys=True)


def summarize(records, model, dry_run):
    groups = collections.defaultdict(list)
    for r in records:
        groups[(r["task"], r["config"])].append(r)
    rows = []
    for (task, cfg), rs in sorted(groups.items()):
        answers = collections.Counter(canonical(r["answer"]) for r in rs)
        modal = answers.most_common(1)[0][1]
        tokens = [r["usage"].get("input_tokens", 0) +
                  r["usage"].get("output_tokens", 0) +
                  r["usage"].get("cache_creation_input_tokens", 0) +
                  r["usage"].get("cache_read_input_tokens", 0) for r in rs]
        costs = [r["cost_usd"] for r in rs if r["cost_usd"] is not None]
        rows.append({
            "task": task, "config": cfg, "runs": len(rs),
            "accuracy": sum(r["correct"] for r in rs) / len(rs),
            "mean_tool_calls": sum(r["tool_calls"] for r in rs) / len(rs),
            "mean_turns": sum(r["turns"] for r in rs) / len(rs),
            "mean_tokens": sum(tokens) / len(rs),
            "mean_input_tokens": sum(r["usage"].get("input_tokens", 0)
                                     for r in rs) / len(rs),
            "mean_output_tokens": sum(r["usage"].get("output_tokens", 0)
                                      for r in rs) / len(rs),
            "mean_cost_usd": sum(costs) / len(costs) if costs else None,
            "mean_wall_seconds": sum(r["wall_seconds"] for r in rs) / len(rs),
            "repeatability": modal / len(rs),
            "stops": dict(collections.Counter(r["stop"] for r in rs)),
        })
    agg = []
    for cfg in sorted({r["config"] for r in rows}):
        cr = [r for r in rows if r["config"] == cfg]
        rs = [r for r in records if r["config"] == cfg]
        costs = [r["cost_usd"] for r in rs if r["cost_usd"] is not None]
        agg.append({
            "config": cfg, "tasks": len(cr), "runs": len(rs),
            "accuracy": sum(r["correct"] for r in rs) / len(rs),
            "mean_tool_calls": sum(r["tool_calls"] for r in rs) / len(rs),
            "mean_turns": sum(r["turns"] for r in rs) / len(rs),
            "mean_tokens": sum(r["mean_tokens"] for r in cr) / len(cr),
            "total_cost_usd": round(sum(costs), 4) if costs else None,
            "mean_wall_seconds": sum(r["wall_seconds"] for r in rs) / len(rs),
            "mean_repeatability": sum(r["repeatability"] for r in cr) / len(cr),
            "fully_repeatable_tasks": sum(r["repeatability"] == 1 for r in cr),
        })
    return {"model": model, "dry_run": dry_run, "per_task": rows,
            "aggregate": agg}


def markdown(summary):
    def f(x, nd=2):
        return "-" if x is None else f"{x:.{nd}f}"
    out = [f"# Agent benchmark: {summary['model']}"
           + (" (dry run, scripted fake model)" if summary["dry_run"] else ""),
           "",
           "A = zorba-mcp tools only; B = bash sandbox only. Tokens include "
           "cache reads and writes; repeatability is the share of runs that "
           "gave the most common answer.", "",
           "## Aggregate", "",
           "| config | tasks | runs | accuracy | tool calls | turns | tokens "
           "| cost USD | wall s | repeatability | fully repeatable tasks |",
           "|---|---|---|---|---|---|---|---|---|---|---|"]
    for a in summary["aggregate"]:
        out.append(
            f"| {a['config']} | {a['tasks']} | {a['runs']} | "
            f"{f(a['accuracy'])} | {f(a['mean_tool_calls'], 1)} | "
            f"{f(a['mean_turns'], 1)} | {f(a['mean_tokens'], 0)} | "
            f"{f(a['total_cost_usd'], 4)} | {f(a['mean_wall_seconds'], 1)} | "
            f"{f(a['mean_repeatability'])} | {a['fully_repeatable_tasks']} |")
    out += ["", "## Per task", "",
            "| task | config | runs | accuracy | tool calls | turns | tokens "
            "| cost USD | wall s | repeatability | stops |",
            "|---|---|---|---|---|---|---|---|---|---|---|"]
    for r in summary["per_task"]:
        stops = ", ".join(f"{k} {v}" for k, v in sorted(r["stops"].items()))
        out.append(
            f"| {r['task']} | {r['config']} | {r['runs']} | "
            f"{f(r['accuracy'])} | {f(r['mean_tool_calls'], 1)} | "
            f"{f(r['mean_turns'], 1)} | {f(r['mean_tokens'], 0)} | "
            f"{f(r['mean_cost_usd'], 4)} | {f(r['mean_wall_seconds'], 1)} | "
            f"{f(r['repeatability'])} | {stops} |")
    return "\n".join(out) + "\n"


# --- main --------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--model", default=DEFAULT_MODEL)
    ap.add_argument("--runs", type=int, default=1)
    ap.add_argument("--config", choices=["A", "B", "both"], default="both")
    ap.add_argument("--tasks", default="",
                    help="comma-separated task ids or prefixes (default: all)")
    ap.add_argument("--dry-run", action="store_true",
                    help="scripted fake model, no API key needed")
    ap.add_argument("--snapshot", default=DEFAULT_SNAPSHOT)
    ap.add_argument("--mcp-command", default="zorba-mcp --root {snapshot}",
                    help="command that starts zorba-mcp ({snapshot} is "
                    "replaced); --timeout and --max-output are appended")
    ap.add_argument("--queries-dir", default="",
                    help="predefined query tools for A; must not contain "
                    "queries that answer a task")
    ap.add_argument("--sandbox-image", default=DEFAULT_IMAGE)
    ap.add_argument("--max-turns", type=int, default=30)
    ap.add_argument("--max-tokens", type=int, default=16000,
                    help="max_tokens of each model response")
    ap.add_argument("--effort", default="",
                    help="output_config.effort (default: the model's)")
    ap.add_argument("--tool-timeout", type=int, default=30)
    ap.add_argument("--max-output", type=int, default=131072)
    ap.add_argument("--out", default=os.path.join(HERE, "results"))
    args = ap.parse_args()

    if not args.dry_run and not (os.environ.get("ANTHROPIC_API_KEY") or
                                 os.environ.get("ANTHROPIC_AUTH_TOKEN")):
        sys.exit("harness.py: ANTHROPIC_API_KEY is not set (use --dry-run "
                 "to test without the API)")
    if args.queries_dir:
        with open(os.path.join(HERE, "tasks", "tasks.json")) as f:
            answering = {t["id"] + ".jq" for t in json.load(f)["tasks"]}
        clash = answering & set(os.listdir(args.queries_dir))
        if clash:
            sys.exit(f"harness.py: --queries-dir holds task answers: {clash}")

    with open(os.path.join(HERE, "tasks", "tasks.json")) as f:
        tasks = json.load(f)["tasks"]
    with open(os.path.join(HERE, "tasks", "ground_truth.json")) as f:
        truth = json.load(f)
    if args.tasks:
        sel = [s.strip() for s in args.tasks.split(",") if s.strip()]
        tasks = [t for t in tasks if any(t["id"].startswith(s) for s in sel)]
    if not tasks:
        sys.exit("harness.py: no task selected")
    configs = ["A", "B"] if args.config == "both" else [args.config]

    os.makedirs(args.out, exist_ok=True)
    runs_path = os.path.join(args.out, "runs.jsonl")
    open(runs_path, "w").close()
    records = []
    lock = threading.Lock()
    for run in range(1, args.runs + 1):
        for task in tasks:
            for cfg in configs:
                cls = ConfigA if cfg == "A" else ConfigB
                config = cls(args, os.path.join(args.out, "zorba-mcp.log"))
                try:
                    model = (FakeModel(task["id"]) if args.dry_run else
                             ClaudeModel(args.model, args.max_tokens,
                                         args.effort))
                    res = run_task(task, config, model, args)
                finally:
                    config.close()
                why = (grading.grade(res["answer"], truth[task["id"]],
                                     task["grading"])
                       if res["answer"] is not None else "no answer")
                rec = {"task": task["id"], "config": cfg, "run": run,
                       "model": "fake" if args.dry_run else args.model,
                       "time": now_iso(), "correct": why is None,
                       "grading": why, **res,
                       "cost_usd": cost_usd(args.model, res["usage"])
                       if not args.dry_run else None}
                records.append(rec)
                with lock, open(runs_path, "a") as f:
                    f.write(json.dumps(rec, default=str) + "\n")
                print(f"run {run} {task['id']} {cfg}: "
                      f"{'correct' if why is None else 'WRONG'} "
                      f"({res['stop']}, {res['tool_calls']} tool calls, "
                      f"{res['wall_seconds']} s)"
                      + ("" if why is None else f": {why}"), flush=True)

    summary = summarize(records, args.model, args.dry_run)
    with open(os.path.join(args.out, "summary.json"), "w") as f:
        json.dump(summary, f, indent=1)
    md = markdown(summary)
    with open(os.path.join(args.out, "summary.md"), "w") as f:
        f.write(md)
    print("\n" + md)
    if args.dry_run and not all(r["correct"] for r in records):
        print("harness.py: dry run: some runs were graded wrong",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
