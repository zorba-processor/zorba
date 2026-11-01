#!/usr/bin/env python3
"""Stand-in for the `claude` executable in the dry run of the claude-code
backend (harness.py --backend claude-code --dry-run).

It accepts only the command line the harness builds for Claude Code and
checks that the fairness options and limits are there (exit status 2
otherwise), reads the prompt from standard input, runs a fixed script with
the tools of its configuration and writes Claude Code's stream-json events
(system/init, assistant and user messages with tool_use and tool_result
blocks, StructuredOutput and the final result event with structured_output,
num_turns, usage and total_cost_usd) to standard output.

  A (--tools ""):   starts the MCP server of --mcp-config and calls
                    list_documents, then run_query with the task's
                    ground-truth query (queries/<task>.jq).
  B (--tools Bash): runs in the sandbox container, like the real claude. It
                    first runs a check of the sandbox through bash: /data is
                    read-only, there is no direct route out, and the egress
                    proxy refuses a feed host and allows the Anthropic API.
                    Then it lists the files and runs reference.py for the
                    task. A failed check ends the run with an error result.

The task id comes from ZORBA_BENCH_STUB_TASK, which the harness sets only in
the dry run.
"""

import argparse
import json
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))

PROBE = r'''
import os, socket, sys
bad = []
try:
    open("/data/.write-probe", "w").close()
    bad.append("/data is writable")
except OSError:
    pass
try:
    socket.create_connection(("1.1.1.1", 443), timeout=5).close()
    bad.append("a direct connection to 1.1.1.1:443 succeeded")
except OSError:
    pass
proxy = os.environ.get("HTTPS_PROXY", "")
def connect(target):
    host, port = proxy.split("//", 1)[1].rstrip("/").rsplit(":", 1)
    s = socket.create_connection((host, int(port)), timeout=10)
    s.sendall(f"CONNECT {target} HTTP/1.1\r\nHost: {target}\r\n\r\n".encode())
    status = s.recv(4096).split(b"\r\n", 1)[0].decode()
    s.close()
    return status
if not proxy:
    bad.append("HTTPS_PROXY is not set")
else:
    st = connect("earthquake.usgs.gov:443")
    if " 403 " not in st:
        bad.append(f"the proxy did not refuse earthquake.usgs.gov: {st}")
    st = connect("api.anthropic.com:443")
    if " 200 " not in st:
        bad.append(f"the proxy did not allow api.anthropic.com: {st}")
if bad:
    print("sandbox check failed: " + "; ".join(bad))
    sys.exit(1)
print("sandbox checks passed")
'''


def fail(msg):
    print(f"stub_claude: {msg}", file=sys.stderr)
    sys.exit(2)


def parse_args():
    ap = argparse.ArgumentParser(prog="claude", add_help=False,
                                 allow_abbrev=False)
    ap.add_argument("-p", action="store_true", required=True)
    ap.add_argument("--model", required=True)
    ap.add_argument("--output-format", required=True,
                    choices=["stream-json"])
    ap.add_argument("--verbose", action="store_true", required=True)
    ap.add_argument("--system-prompt", required=True)
    ap.add_argument("--max-turns", type=int, required=True)
    ap.add_argument("--json-schema", required=True)
    ap.add_argument("--permission-mode", required=True, choices=["dontAsk"])
    ap.add_argument("--strict-mcp-config", action="store_true",
                    required=True)
    ap.add_argument("--no-session-persistence", action="store_true",
                    required=True)
    ap.add_argument("--effort")
    ap.add_argument("--tools", required=True)
    ap.add_argument("--mcp-config")
    ap.add_argument("--allowedTools", required=True)
    ap.add_argument("--settings")
    return ap.parse_args()


def check_env(names):
    for n in names:
        if not os.environ.get(n):
            fail(f"{n} is not set")


class Out:
    def __init__(self, model):
        self.session = "00000000-0000-4000-8000-000000000000"
        self.model = model
        self.n = 0
        self.turns = 0

    def emit(self, ev):
        ev.setdefault("session_id", self.session)
        sys.stdout.write(json.dumps(ev) + "\n")
        sys.stdout.flush()

    def usage(self):
        return {"input_tokens": 100, "output_tokens": 50,
                "cache_creation_input_tokens": 0,
                "cache_read_input_tokens": 0}

    def tool_use(self, name, inp):
        self.n += 1
        self.turns += 1
        tid = f"toolu_stub_{self.n}"
        self.emit({"type": "assistant", "parent_tool_use_id": None,
                   "message": {
                       "id": f"msg_stub_{self.n}", "type": "message",
                       "role": "assistant", "model": self.model,
                       "content": [{"type": "tool_use", "id": tid,
                                    "name": name, "input": inp}],
                       "usage": self.usage()}})
        return tid

    def tool_result(self, tid, text, is_error):
        self.emit({"type": "user", "parent_tool_use_id": None,
                   "message": {"role": "user", "content": [{
                       "type": "tool_result", "tool_use_id": tid,
                       "content": text, "is_error": is_error}]}})

    def result(self, start, subtype, answer=None, errors=None):
        ev = {"type": "result", "subtype": subtype,
              "is_error": subtype != "success",
              "duration_ms": int((time.monotonic() - start) * 1000),
              "duration_api_ms": 0, "num_turns": self.turns,
              "stop_reason": "end_turn", "total_cost_usd": 0.0,
              "usage": {k: v * self.turns for k, v in self.usage().items()},
              "modelUsage": {}, "permission_denials": []}
        if subtype == "success":
            ev["result"] = ""
            ev["structured_output"] = {"answer": answer}
        else:
            ev["errors"] = errors or []
        self.emit(ev)


def run_bash(command):
    timeout = int(os.environ["BASH_DEFAULT_TIMEOUT_MS"]) / 1000
    limit = int(os.environ["BASH_MAX_OUTPUT_LENGTH"])
    try:
        proc = subprocess.run(["bash", "-c", command], capture_output=True,
                              text=True, timeout=timeout,
                              stdin=subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        return f"Command timed out after {timeout} s", True
    out = (proc.stdout + proc.stderr)[:limit]
    return out, proc.returncode != 0


def main():
    args = parse_args()
    task = os.environ.get("ZORBA_BENCH_STUB_TASK") or fail(
        "ZORBA_BENCH_STUB_TASK is not set")
    prompt = sys.stdin.read()
    schema = json.loads(args.json_schema)
    if schema.get("required") != ["answer"] or "answer" not in prompt:
        fail("the prompt or --json-schema is not the harness's")
    if "structured output" not in args.system_prompt:
        fail("the system prompt does not ask for the structured output")
    for n in ("ANTHROPIC_API_KEY", "ANTHROPIC_AUTH_TOKEN"):
        if n in os.environ:
            fail(f"{n} must not reach claude (it outranks the OAuth token)")
    check_env(["CLAUDE_CODE_DISABLE_CLAUDE_MDS",
               "CLAUDE_CODE_DISABLE_AUTO_MEMORY",
               "CLAUDE_CODE_MAX_OUTPUT_TOKENS"])
    out = Out(args.model)
    start = time.monotonic()

    if args.tools == "":
        if args.allowedTools != "mcp__zorba" or not args.mcp_config:
            fail("configuration A needs --mcp-config and "
                 "--allowedTools mcp__zorba")
        check_env(["MCP_TOOL_TIMEOUT", "MAX_MCP_OUTPUT_TOKENS",
                   "CLAUDE_CONFIG_DIR"])
        with open(args.mcp_config) as f:
            servers = json.load(f)["mcpServers"]
        if list(servers) != ["zorba"]:
            fail(f"expected one MCP server named zorba, got {list(servers)}")
        srv = servers["zorba"]
        sys.path.insert(0, HERE)
        from harness import MCPClient
        client = MCPClient([srv["command"]] + srv["args"], os.devnull)
        try:
            names = [t["name"] for t in client.tools]
            out.emit({"type": "system", "subtype": "init",
                      "model": args.model, "permissionMode": "dontAsk",
                      "apiKeySource": "none",
                      "claude_code_version": "stub",
                      "tools": ["StructuredOutput"] +
                      [f"mcp__zorba__{n}" for n in names],
                      "mcp_servers": [{"name": "zorba",
                                       "status": "connected"}]})
            with open(os.path.join(HERE, "queries", task + ".jq")) as f:
                query = f.read()
            text = ""
            for name, inp in (("list_documents", {}),
                              ("run_query", {"query": query})):
                tid = out.tool_use("mcp__zorba__" + name, inp)
                text, is_error = client.call(name, inp)
                out.tool_result(tid, text, is_error)
        finally:
            client.close()
    elif args.tools == "Bash":
        if args.allowedTools != "Bash" or args.mcp_config:
            fail("configuration B needs --allowedTools Bash and no MCP "
                 "server")
        check_env(["BASH_DEFAULT_TIMEOUT_MS", "BASH_MAX_TIMEOUT_MS",
                   "CLAUDE_CODE_DISABLE_BACKGROUND_TASKS",
                   "BASH_MAX_OUTPUT_LENGTH", "HOME", "HTTPS_PROXY"])
        settings = json.loads(args.settings or "{}")
        if settings.get("bashOutputMaxChars") != int(
                os.environ["BASH_MAX_OUTPUT_LENGTH"]):
            fail("bashOutputMaxChars does not match BASH_MAX_OUTPUT_LENGTH")
        out.emit({"type": "system", "subtype": "init", "model": args.model,
                  "permissionMode": "dontAsk", "apiKeySource": "none",
                  "claude_code_version": "stub",
                  "tools": ["Bash", "StructuredOutput"], "mcp_servers": []})
        with open("/bench/reference.py") as f:
            ref = ("__file__ = '/tmp/reference.py'\n" + f.read().replace(
                'if __name__ == "__main__":', "if False:"))
        script = [
            "python3 - <<'PYEOF'\n" + PROBE + "\nPYEOF",
            "find . -type f | sort | head -50",
            "python3 - <<'PYEOF'\n" + ref + "\nimport json\n"
            f"print(json.dumps(compute('/data', ['{task}'])['{task}']))\n"
            "PYEOF",
        ]
        text = ""
        for i, command in enumerate(script):
            tid = out.tool_use("Bash", {"command": command})
            text, is_error = run_bash(command)
            out.tool_result(tid, text, is_error)
            if i == 0 and "sandbox checks passed" not in text:
                out.result(start, "error_during_execution",
                           errors=[text.strip()])
                return 1
    else:
        fail(f"unexpected --tools {args.tools!r}")

    if out.turns + 1 > args.max_turns:
        out.result(start, "error_max_turns")
        return 1
    lines = [ln for ln in text.splitlines() if ln.strip()]
    try:
        answer = json.loads(lines[-1])
    except (IndexError, ValueError):
        answer = {"unparsable_tool_output": text[:200]}
    tid = out.tool_use("StructuredOutput", {"answer": answer})
    out.tool_result(tid, "Structured output provided successfully.", False)
    out.result(start, "success", answer=answer)
    return 0


if __name__ == "__main__":
    sys.exit(main())
