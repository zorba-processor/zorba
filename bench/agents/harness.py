#!/usr/bin/env python3
"""Agent benchmark harness: the same model answers the same tasks over the
same snapshot directory with two tool configurations.

  A  the zorba-mcp tools only (list_documents, run_query, and the predefined
     query tools of --queries-dir, if any), over MCP/stdio
  B  a bash tool only, in a container with the snapshot mounted read-only,
     no network except as stated below and CPU/memory/time limits (python3,
     jq, xmllint, grep, awk)

Two backends drive the model; each runner supplies their own credentials:

  api          the Anthropic Messages API through the Python SDK (manual tool
               loop); needs ANTHROPIC_API_KEY in the environment
  claude-code  Claude Code headless (claude -p); needs CLAUDE_CODE_OAUTH_TOKEN
               (from `claude setup-token`). In A, claude runs on the host
               with no built-in tools and zorba-mcp as its only MCP server; in
               B, claude runs inside the sandbox container with the Bash tool
               only, and the container's only network route is an egress
               proxy that allows the Anthropic API and nothing else.

Within a backend both configurations get the same system prompt, the same
per-task prompt, the same turn and token budget and the same way of giving
the final answer (the submit_answer tool for api, structured output with
--json-schema for claude-code); only the data tools differ. Every run is
graded against tasks/ground_truth.json.

Output (--out): runs.jsonl (one line per run, with the tool-call log),
summary.json and summary.md (per task and per configuration: accuracy, mean
tool calls, turns, tokens, cost estimate, reported cost, wall time,
repeatability).

--dry-run replaces the model with a script and needs no credentials: in A it
runs the task's ground-truth JSONiq query (queries/<task>.jq) through
zorba-mcp's run_query tool, in B it runs the Python reference (reference.py)
through the bash tool inside the sandbox, and gives the result as the
answer. For api the script stands in for the SDK responses; for claude-code
it is stub_claude.py, a stand-in `claude` executable that receives the same
command line, environment and container as the real one and writes the
stream-json events Claude Code writes. Every dry run must be graded correct.
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
import tempfile
import threading
import time
import uuid

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import grading  # noqa: E402

DEFAULT_MODEL = "claude-sonnet-5-5"
DEFAULT_SNAPSHOT = os.path.join(HERE, "data", "snapshot")
DEFAULT_IMAGE = "zorba-agent-sandbox:latest"
DEFAULT_CLAUDE_IMAGE = "zorba-agent-sandbox-claude:latest"
STUB_CLAUDE = os.path.join(HERE, "stub_claude.py")
EGRESS_PROXY = os.path.join(HERE, "egress_proxy.py")

# Claude Code saves a successful MCP tool result of more than 50,000
# characters to a file and gives the model a preview; configuration A has no
# tool to read that file. The claude-code backend therefore caps every tool
# result (zorba-mcp --max-output and the Bash tool) at this many bytes or
# characters, below that threshold, in both configurations.
CLAUDE_CODE_MAX_OUTPUT = 50000

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
{answer_rule}"""

ANSWER_RULES = {
    "api": "- When you have the answer, call the submit_answer tool once, "
           "with the answer in the JSON shape given with the question. Do not "
           "call it before you are done.",
    "claude-code": "- When you have the answer, give it once as your "
                   "structured output, in the JSON shape given with the "
                   "question. Do not give it before you are done.",
}

ANSWER_HINTS = {
    "api": "the \"answer\" argument of submit_answer",
    "claude-code": "the \"answer\" member of the structured output",
}


def system_prompt(backend):
    return SYSTEM_PROMPT.format(answer_rule=ANSWER_RULES[backend])


def task_prompt(task, backend):
    return (task["question"] + "\n\nAnswer JSON schema (" +
            ANSWER_HINTS[backend] + "):\n" +
            json.dumps(task["answer_schema"]))


def answer_schema(task):
    """The JSON schema of the final answer, wrapped like the input of the
    submit_answer tool."""
    return {"type": "object",
            "properties": {"answer": task["answer_schema"]},
            "required": ["answer"]}

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


def mcp_command(args):
    """The command line that starts zorba-mcp, with the per-call limits."""
    cmd = [part.format(snapshot=args.snapshot, queries=args.queries_dir or "")
           for part in shlex.split(args.mcp_command)]
    if args.queries_dir:
        cmd += ["--queries", args.queries_dir]
    return cmd + ["--timeout", str(args.tool_timeout),
                  "--max-output", str(args.max_output)]


class ConfigA:
    name = "A"

    def __init__(self, args, log_path):
        self.client = MCPClient(mcp_command(args), log_path)
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
              "input_schema": answer_schema(task)}
    tools = config.tools + [submit]
    prompt = task_prompt(task, "api")
    system = system_prompt("api")
    messages = [{"role": "user", "content": prompt}]
    usage = collections.Counter()
    log, answer, stop, text = [], None, "turn_limit", ""
    turns = tool_calls = 0
    start = time.monotonic()
    while turns < args.max_turns:
        turns += 1
        try:
            resp = model.create(system, tools, messages)
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
        "tools_available": [t["name"] for t in tools],
        # Only Claude Code reports these.
        "reported_cost_usd": None, "duration_api_ms": None,
        "permission_denials": None,
    }


# --- claude-code backend -----------------------------------------------------
#
# Fairness is enforced by Claude Code flags, environment and the container,
# never by prompt text:
#
#   both  --model, --system-prompt (replaces Claude Code's own prompt),
#         --max-turns, --json-schema (structured output), --permission-mode
#         dontAsk (nothing prompts; a tool outside --allowedTools is denied),
#         --strict-mcp-config, a fresh configuration directory, CLAUDE.md and
#         auto memory disabled, CLAUDE_CODE_MAX_OUTPUT_TOKENS, and no
#         ANTHROPIC_API_KEY / ANTHROPIC_AUTH_TOKEN (they would take
#         precedence over the OAuth token).
#   A     --tools "" (no built-in tool: no Bash, Read, Grep, Glob, WebFetch,
#         WebSearch), --mcp-config with zorba-mcp as the only server,
#         --allowedTools mcp__zorba; zorba-mcp enforces the per-call time
#         and output limits.
#   B     --tools Bash --allowedTools Bash; claude runs inside the sandbox
#         container (snapshot read-only, read-only root file system, CPU,
#         memory and process limits) on an internal Docker network whose only
#         way out is egress_proxy.py, which allows the Anthropic API only;
#         BASH_DEFAULT_TIMEOUT_MS = BASH_MAX_TIMEOUT_MS = the tool timeout,
#         CLAUDE_CODE_DISABLE_BACKGROUND_TASKS (a command that times out is
#         stopped, not moved to the background), BASH_MAX_OUTPUT_LENGTH and
#         bashOutputMaxChars = the output limit.
#
# The harness also checks, from the init event of every run, that the tools
# Claude Code offers the model are exactly the expected ones, and records a
# run with any other tool set as "tool_set_mismatch" (graded wrong).

STRUCTURED_OUTPUT_TOOL = "StructuredOutput"
MCP_SERVER_NAME = "zorba"


def claude_env(args, cfg):
    """Environment variables set for claude, the same in A and B except the
    tool-specific limits, which apply to the one tool each configuration
    has."""
    ms = str(args.tool_timeout * 1000)
    env = {
        "CLAUDE_CODE_DISABLE_CLAUDE_MDS": "1",
        "CLAUDE_CODE_DISABLE_AUTO_MEMORY": "1",
        "CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC": "1",
        "DISABLE_AUTOUPDATER": "1",
        "CLAUDE_CODE_MAX_OUTPUT_TOKENS": str(args.max_tokens),
    }
    if cfg == "A":
        # zorba-mcp enforces --timeout and --max-output itself; these only
        # keep Claude Code from cutting earlier (a backstop of 30 s more for
        # the call, and a token limit no smaller than the byte limit).
        env["MCP_TOOL_TIMEOUT"] = str((args.tool_timeout + 30) * 1000)
        env["MAX_MCP_OUTPUT_TOKENS"] = str(args.max_output)
    else:
        env["BASH_DEFAULT_TIMEOUT_MS"] = ms
        env["BASH_MAX_TIMEOUT_MS"] = ms
        env["CLAUDE_CODE_DISABLE_BACKGROUND_TASKS"] = "1"
        env["BASH_MAX_OUTPUT_LENGTH"] = str(args.max_output)
    return env


def claude_args(task, cfg, args, mcp_config_path=None):
    """The claude command-line options (without the executable)."""
    out = ["-p", "--model", args.model,
           "--output-format", "stream-json", "--verbose",
           "--system-prompt", system_prompt("claude-code"),
           "--max-turns", str(args.max_turns),
           "--json-schema", json.dumps(answer_schema(task)),
           "--permission-mode", "dontAsk",
           "--strict-mcp-config",
           "--no-session-persistence"]
    if args.effort:
        out += ["--effort", args.effort]
    if cfg == "A":
        out += ["--tools", "", "--mcp-config", mcp_config_path,
                "--allowedTools", "mcp__" + MCP_SERVER_NAME]
    else:
        out += ["--tools", "Bash", "--allowedTools", "Bash",
                "--settings", json.dumps(
                    {"bashOutputMaxChars": args.max_output})]
    return out


def expected_tools(cfg, mcp_tools):
    base = {STRUCTURED_OUTPUT_TOOL}
    if cfg == "A":
        return base | {f"mcp__{MCP_SERVER_NAME}__{t}" for t in mcp_tools}
    return base | {"Bash"}


class ClaudeNetwork:
    """The internal Docker network of configuration B and its egress proxy
    (one per harness invocation; every run gets a fresh claude container)."""

    def __init__(self, args, log_path):
        tag = uuid.uuid4().hex[:12]
        self.net = f"zorba-agent-net-{tag}"
        self.proxy = f"zorba-agent-proxy-{tag}"
        self.log_path = log_path
        subprocess.run(["docker", "network", "create", "--internal",
                        self.net], check=True, capture_output=True)
        allow = []
        for a in args.egress_allow.split(","):
            if a.strip():
                allow += ["--allow", a.strip()]
        subprocess.run(
            ["docker", "run", "-d", "--rm", "--name", self.proxy,
             "--read-only", "--cap-drop", "ALL",
             "--security-opt", "no-new-privileges",
             "-v", f"{EGRESS_PROXY}:/proxy/egress_proxy.py:ro",
             "--entrypoint", "python3", args.sandbox_image,
             "-I", "/proxy/egress_proxy.py", *allow],
            check=True, capture_output=True)
        subprocess.run(["docker", "network", "connect", self.net,
                        self.proxy], check=True, capture_output=True)
        self.url = f"http://{self.proxy}:3128"

    def close(self):
        logs = subprocess.run(["docker", "logs", self.proxy],
                              capture_output=True, text=True)
        with open(self.log_path, "a") as f:
            f.write(logs.stdout + logs.stderr)
        subprocess.run(["docker", "rm", "-f", self.proxy],
                       capture_output=True)
        subprocess.run(["docker", "network", "rm", self.net],
                       capture_output=True)


def claude_command(args, cfg, task, env, workdir, network):
    """The full command that runs claude for one task, the environment of
    that command, and the name of its container (B) or None (A)."""
    base_env = {k: v for k, v in os.environ.items()
                if k not in ("ANTHROPIC_API_KEY", "ANTHROPIC_AUTH_TOKEN")}
    if args.dry_run:
        env = dict(env, ZORBA_BENCH_STUB_TASK=task["id"])
    if cfg == "A":
        mcp_cmd = mcp_command(args)
        mcp_path = os.path.join(workdir, "mcp.json")
        with open(mcp_path, "w") as f:
            json.dump({"mcpServers": {MCP_SERVER_NAME: {
                "type": "stdio", "command": mcp_cmd[0],
                "args": mcp_cmd[1:]}}}, f)
        exe = ([sys.executable, "-I", STUB_CLAUDE] if args.dry_run
               else shlex.split(args.claude_command))
        full_env = dict(base_env, **env,
                        CLAUDE_CONFIG_DIR=os.path.join(workdir, "config"))
        return exe + claude_args(task, cfg, args, mcp_path), full_env, None
    snapshot = os.path.abspath(args.snapshot)
    name = f"zorba-agent-claude-{uuid.uuid4().hex[:12]}"
    run = ["docker", "run", "--rm", "-i", "--name", name,
           "--network", network.net,
           "--read-only", "--tmpfs", "/tmp:rw,size=64m",
           "--tmpfs", "/home/agent:rw,size=256m,uid=1000,gid=1000,mode=0700",
           "--cpus", "1", "--memory", "2g", "--pids-limit", "512",
           "--security-opt", "no-new-privileges", "--cap-drop", "ALL",
           "-v", f"{snapshot}:/data:ro", "-w", "/data",
           "-e", "HOME=/home/agent",
           "-e", f"HTTPS_PROXY={network.url}",
           "-e", f"https_proxy={network.url}",
           "-e", "CLAUDE_CODE_OAUTH_TOKEN"]
    for k, v in env.items():
        run += ["-e", f"{k}={v}"]
    if args.dry_run:
        run += ["-v", f"{HERE}:/bench:ro", args.claude_image,
                "python3", "-I", "/bench/stub_claude.py"]
    else:
        run += [args.claude_image, "claude"]
    return run + claude_args(task, cfg, args), base_env, name


def parse_stream(lines):
    """Reads Claude Code's stream-json output: the init event, the assistant
    and user messages (tool calls and their results) and the result event."""
    init, result, uses, results = None, None, [], {}
    turn_ids, text = set(), ""
    for line in lines:
        line = line.strip()
        if not line:
            continue
        try:
            ev = json.loads(line)
        except ValueError:
            continue
        kind = ev.get("type")
        if kind == "system" and ev.get("subtype") == "init":
            init = ev
        elif kind == "result":
            result = ev
        elif kind in ("assistant", "user"):
            if ev.get("parent_tool_use_id"):
                continue
            msg = ev.get("message") or {}
            if kind == "assistant" and msg.get("id"):
                turn_ids.add(msg["id"])
            content = msg.get("content")
            if not isinstance(content, list):
                continue
            for b in content:
                if not isinstance(b, dict):
                    continue
                if b.get("type") == "tool_use":
                    uses.append(b)
                elif b.get("type") == "text" and kind == "assistant":
                    text += b.get("text", "")
                elif b.get("type") == "tool_result":
                    c = b.get("content")
                    if isinstance(c, list):
                        c = "\n".join(x.get("text", "") for x in c
                                      if isinstance(x, dict))
                    results[b.get("tool_use_id")] = (
                        c if isinstance(c, str) else json.dumps(c),
                        bool(b.get("is_error")))
    return {"init": init, "result": result, "uses": uses,
            "results": results, "turn_ids": turn_ids, "text": text}


def mcp_tool_names(args, log_path):
    """The tools zorba-mcp offers, to check the tool set of configuration A
    against."""
    client = MCPClient(mcp_command(args), log_path)
    try:
        return [t["name"] for t in client.tools]
    finally:
        client.close()


def run_task_claude(task, cfg, args, network, mcp_tools, log_dir):
    env = claude_env(args, cfg)
    log, answer, stop = [], None, "no_answer"
    start = time.monotonic()
    with tempfile.TemporaryDirectory(prefix="zorba-agent-") as workdir:
        cmd, full_env, container = claude_command(args, cfg, task, env,
                                                  workdir, network)
        try:
            proc = subprocess.run(
                cmd, input=task_prompt(task, "claude-code"),
                capture_output=True, text=True, env=full_env, cwd=workdir,
                timeout=args.run_timeout)
            out, err, code = proc.stdout, proc.stderr, proc.returncode
        except subprocess.TimeoutExpired as e:
            out = e.stdout.decode() if isinstance(e.stdout, bytes) else (
                e.stdout or "")
            err, code = f"run timed out after {args.run_timeout} s", None
        finally:
            if container:
                subprocess.run(["docker", "rm", "-f", container],
                               capture_output=True)
    wall = time.monotonic() - start
    with open(os.path.join(log_dir, "claude-stderr.log"), "a") as f:
        f.write(f"--- {task['id']} {cfg}: exit {code}\n{err}\n")
    s = parse_stream(out.splitlines())
    init, result = s["init"] or {}, s["result"] or {}
    tools = list(init.get("tools") or [])
    for u in s["uses"]:
        if u.get("name") == STRUCTURED_OUTPUT_TOOL:
            continue
        res, is_error = s["results"].get(u.get("id"), ("", False))
        log.append({"tool": u.get("name"), "input": u.get("input"),
                    "output": res[:4000], "output_bytes": len(res),
                    "is_error": is_error, "seconds": None})
    sub = result.get("subtype")
    if code is None:
        stop = "run_timeout"
    elif not result:
        stop = f"claude_exit_{code}"
    elif set(tools) != expected_tools(cfg, mcp_tools):
        stop = "tool_set_mismatch"
    elif result.get("stop_reason") == "refusal":
        stop = "refusal"
    elif sub == "success":
        so = result.get("structured_output")
        if isinstance(so, dict) and "answer" in so:
            answer, stop = so["answer"], "submitted"
        else:
            answer = extract_json_answer(result.get("result") or s["text"])
            stop = "answer_in_text" if answer is not None else "no_answer"
    elif sub == "error_max_turns":
        stop = "turn_limit"
    else:
        stop = sub or "unknown_result"
    usage = result.get("usage") or {}
    return {
        "answer": answer, "stop": stop,
        "turns": result.get("num_turns", len(s["turn_ids"])),
        "tool_calls": len(log),
        "usage": {k: usage.get(k, 0) or 0 for k in (
            "input_tokens", "output_tokens", "cache_creation_input_tokens",
            "cache_read_input_tokens")},
        "wall_seconds": round(wall, 2), "tool_log": log,
        "final_text": (result.get("result") or s["text"] or "")[-2000:],
        "tools_available": tools,
        "reported_cost_usd": result.get("total_cost_usd"),
        "duration_api_ms": result.get("duration_api_ms"),
        "permission_denials": result.get("permission_denials"),
    }


CLI_ERRORS = re.compile(
    r"unknown option|error: option|too many arguments|invalid choice|"
    r"Allowed choices|not a valid JSON Schema|missing required argument",
    re.I)


def check_cli(args, tasks):
    """Runs the real claude of --claude-image, without credentials and
    without network, with the command line of each configuration, and
    fails if Claude Code rejects any option (it cannot get further than the
    authentication check). Returns the number of rejected command lines."""
    bad = 0
    mcp = json.dumps({"mcpServers": {MCP_SERVER_NAME: {
        "type": "stdio", "command": "true", "args": []}}})
    for cfg in ("A", "B"):
        argv = claude_args(tasks[0], cfg, args, "/tmp/mcp.json")
        run = ["docker", "run", "--rm", "-i", "--network", "none",
               "--tmpfs", "/home/agent:rw,uid=1000,gid=1000",
               "-e", "HOME=/home/agent"]
        for k, v in claude_env(args, cfg).items():
            run += ["-e", f"{k}={v}"]
        run += ["--entrypoint", "bash", args.claude_image, "-c",
                'printf %s "$0" > /tmp/mcp.json && exec claude "$@"', mcp]
        try:
            proc = subprocess.run(run + argv, input="Which?",
                                  capture_output=True, text=True,
                                  timeout=120)
            text = proc.stdout + proc.stderr
        except subprocess.TimeoutExpired:
            text, proc = "timed out after 120 s", None
        rejected = proc is None or proc.returncode == 0 or \
            bool(CLI_ERRORS.search(text))
        print(f"claude command line of {cfg}: "
              f"{'REJECTED' if rejected else 'accepted'} "
              f"(exit {proc.returncode if proc else None}): "
              f"{text.strip()[-400:]}")
        bad += rejected
    return bad


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


def mean_or_none(xs):
    xs = [x for x in xs if x is not None]
    return sum(xs) / len(xs) if xs else None


def summarize(records, model, dry_run, backend):
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
            "mean_cache_creation_input_tokens": sum(
                r["usage"].get("cache_creation_input_tokens", 0)
                for r in rs) / len(rs),
            "mean_cache_read_input_tokens": sum(
                r["usage"].get("cache_read_input_tokens", 0)
                for r in rs) / len(rs),
            "mean_cost_usd": sum(costs) / len(costs) if costs else None,
            "mean_reported_cost_usd": mean_or_none(
                r.get("reported_cost_usd") for r in rs),
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
            "total_reported_cost_usd": (
                round(sum(r["reported_cost_usd"] for r in rs
                          if r.get("reported_cost_usd") is not None), 4)
                if any(r.get("reported_cost_usd") is not None for r in rs)
                else None),
            "mean_wall_seconds": sum(r["wall_seconds"] for r in rs) / len(rs),
            "mean_repeatability": sum(r["repeatability"] for r in cr) / len(cr),
            "fully_repeatable_tasks": sum(r["repeatability"] == 1 for r in cr),
        })
    return {"model": model, "backend": backend, "dry_run": dry_run,
            "per_task": rows, "aggregate": agg}


def markdown(summary):
    def f(x, nd=2):
        return "-" if x is None else f"{x:.{nd}f}"
    out = [f"# Agent benchmark: {summary['model']} "
           f"(backend {summary['backend']})"
           + (" (dry run, scripted fake model)" if summary["dry_run"] else ""),
           "",
           "A = zorba-mcp tools only; B = bash sandbox only. Tokens include "
           "cache reads and writes; repeatability is the share of runs that "
           "gave the most common answer. Cost USD is the harness estimate "
           "from list prices; reported USD is the total_cost_usd that Claude "
           "Code reports (claude-code backend only; an estimate, not "
           "billing).", "",
           "## Aggregate", "",
           "| config | tasks | runs | accuracy | tool calls | turns | tokens "
           "| cost USD | wall s | repeatability | fully repeatable tasks "
           "| reported USD |",
           "|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for a in summary["aggregate"]:
        out.append(
            f"| {a['config']} | {a['tasks']} | {a['runs']} | "
            f"{f(a['accuracy'])} | {f(a['mean_tool_calls'], 1)} | "
            f"{f(a['mean_turns'], 1)} | {f(a['mean_tokens'], 0)} | "
            f"{f(a['total_cost_usd'], 4)} | {f(a['mean_wall_seconds'], 1)} | "
            f"{f(a['mean_repeatability'])} | {a['fully_repeatable_tasks']} | "
            f"{f(a['total_reported_cost_usd'], 4)} |")
    out += ["", "## Per task", "",
            "| task | config | runs | accuracy | tool calls | turns | tokens "
            "| cost USD | wall s | repeatability | stops | reported USD |",
            "|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for r in summary["per_task"]:
        stops = ", ".join(f"{k} {v}" for k, v in sorted(r["stops"].items()))
        out.append(
            f"| {r['task']} | {r['config']} | {r['runs']} | "
            f"{f(r['accuracy'])} | {f(r['mean_tool_calls'], 1)} | "
            f"{f(r['mean_turns'], 1)} | {f(r['mean_tokens'], 0)} | "
            f"{f(r['mean_cost_usd'], 4)} | {f(r['mean_wall_seconds'], 1)} | "
            f"{f(r['repeatability'])} | {stops} | "
            f"{f(r['mean_reported_cost_usd'], 4)} |")
    return "\n".join(out) + "\n"


# --- main --------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--backend", choices=["api", "claude-code"],
                    default="api",
                    help="api: Anthropic SDK with ANTHROPIC_API_KEY; "
                    "claude-code: claude -p with CLAUDE_CODE_OAUTH_TOKEN")
    ap.add_argument("--model", default=DEFAULT_MODEL)
    ap.add_argument("--runs", type=int, default=1)
    ap.add_argument("--config", choices=["A", "B", "both"], default="both")
    ap.add_argument("--tasks", default="",
                    help="comma-separated task ids or prefixes (default: all)")
    ap.add_argument("--dry-run", action="store_true",
                    help="scripted fake model, no credentials needed")
    ap.add_argument("--snapshot", default=DEFAULT_SNAPSHOT)
    ap.add_argument("--mcp-command", default="zorba-mcp --root {snapshot}",
                    help="command that starts zorba-mcp ({snapshot} is "
                    "replaced); --timeout and --max-output are appended")
    ap.add_argument("--queries-dir", default="",
                    help="predefined query tools for A; must not contain "
                    "queries that answer a task")
    ap.add_argument("--sandbox-image", default=DEFAULT_IMAGE,
                    help="bash sandbox of B (api), and the image of the "
                    "egress proxy (claude-code)")
    ap.add_argument("--claude-image", default=DEFAULT_CLAUDE_IMAGE,
                    help="claude-code, B: the sandbox image with Claude Code")
    ap.add_argument("--claude-command", default="claude",
                    help="claude-code, A: how to run Claude Code on the host")
    ap.add_argument("--egress-allow", default="api.anthropic.com:443",
                    help="claude-code, B: comma-separated host:port pairs "
                    "the egress proxy allows")
    ap.add_argument("--check-cli", action="store_true",
                    help="claude-code: check that the real claude of "
                    "--claude-image accepts the command line of both "
                    "configurations (no credentials, no network), and exit")
    ap.add_argument("--run-timeout", type=int, default=1800,
                    help="claude-code: seconds before one run is stopped")
    ap.add_argument("--max-turns", type=int, default=30)
    ap.add_argument("--max-tokens", type=int, default=16000,
                    help="max_tokens of each model response "
                    "(CLAUDE_CODE_MAX_OUTPUT_TOKENS for claude-code)")
    ap.add_argument("--effort", default="",
                    help="output_config.effort, or claude --effort "
                    "(default: the model's)")
    ap.add_argument("--tool-timeout", type=int, default=30)
    ap.add_argument("--max-output", type=int, default=None,
                    help="per-call output limit (default 131072 bytes for "
                    f"api, {CLAUDE_CODE_MAX_OUTPUT} for claude-code, which "
                    "refuses more)")
    ap.add_argument("--out", default=os.path.join(HERE, "results"))
    args = ap.parse_args()

    if args.max_output is None:
        args.max_output = (131072 if args.backend == "api"
                           else CLAUDE_CODE_MAX_OUTPUT)
    if args.backend == "claude-code" and \
            args.max_output > CLAUDE_CODE_MAX_OUTPUT:
        sys.exit(f"harness.py: --max-output above {CLAUDE_CODE_MAX_OUTPUT} "
                 "would let Claude Code save MCP results to files that "
                 "configuration A cannot read")
    if args.check_cli:
        with open(os.path.join(HERE, "tasks", "tasks.json")) as f:
            return 1 if check_cli(args, json.load(f)["tasks"]) else 0
    if not args.dry_run:
        if args.backend == "api" and not os.environ.get("ANTHROPIC_API_KEY"):
            sys.exit("harness.py: --backend api needs your own "
                     "ANTHROPIC_API_KEY in the environment (use --dry-run to "
                     "test without credentials)")
        if args.backend == "claude-code" and \
                not os.environ.get("CLAUDE_CODE_OAUTH_TOKEN"):
            sys.exit("harness.py: --backend claude-code needs "
                     "CLAUDE_CODE_OAUTH_TOKEN in the environment (create it "
                     "with `claude setup-token`; use --dry-run to test "
                     "without credentials)")
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
    mcp_log = os.path.join(args.out, "zorba-mcp.log")
    network, mcp_tools = None, []
    if args.backend == "claude-code":
        if "A" in configs:
            mcp_tools = mcp_tool_names(args, mcp_log)
        if "B" in configs:
            network = ClaudeNetwork(
                args, os.path.join(args.out, "egress-proxy.log"))
    try:
        status = run_all(args, tasks, configs, truth, records, lock,
                         runs_path, mcp_log, network, mcp_tools)
    finally:
        if network:
            network.close()
    if status:
        return status

    summary = summarize(records, args.model, args.dry_run, args.backend)
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


def run_all(args, tasks, configs, truth, records, lock, runs_path, mcp_log,
            network, mcp_tools):
    for run in range(1, args.runs + 1):
        for task in tasks:
            for cfg in configs:
                if args.backend == "claude-code":
                    res = run_task_claude(task, cfg, args, network, mcp_tools,
                                          args.out)
                else:
                    cls = ConfigA if cfg == "A" else ConfigB
                    config = cls(args, mcp_log)
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
                       "backend": args.backend,
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
                if res["stop"] == "tool_set_mismatch":
                    print(f"harness.py: configuration {cfg} offered the "
                          f"tools {sorted(res['tools_available'])}, expected "
                          f"{sorted(expected_tools(cfg, mcp_tools))}; "
                          "stopping", file=sys.stderr)
                    return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
