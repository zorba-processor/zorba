"""Unit tests of the claude-code backend of harness.py: the command line
and environment that enforce the fairness rules, the stream-json reader, the
stand-in claude (stub_claude.py) accepting exactly that command line, and
the egress proxy."""

import argparse
import json
import socket
import socketserver
import sys
import threading
import unittest
from unittest import mock

import egress_proxy
import harness
import stub_claude


def args(**kw):
    base = dict(model="claude-sonnet-5-5", max_turns=30, max_tokens=16000,
                effort="", tool_timeout=30,
                max_output=harness.CLAUDE_CODE_MAX_OUTPUT, dry_run=False)
    base.update(kw)
    return argparse.Namespace(**base)


TASK = {"id": "t01_x", "question": "Which?",
        "answer_schema": {"type": "object",
                          "properties": {"id": {"type": "string"}}}}


def opt(argv, name):
    return argv[argv.index(name) + 1]


class ClaudeArgsTest(unittest.TestCase):
    def test_common(self):
        for cfg in ("A", "B"):
            a = harness.claude_args(TASK, cfg, args(), "/tmp/mcp.json")
            self.assertEqual(opt(a, "--model"), "claude-sonnet-5-5")
            self.assertEqual(opt(a, "--permission-mode"), "dontAsk")
            self.assertEqual(opt(a, "--max-turns"), "30")
            self.assertEqual(opt(a, "--output-format"), "stream-json")
            self.assertIn("--strict-mcp-config", a)
            self.assertEqual(opt(a, "--system-prompt"),
                             harness.system_prompt("claude-code"))
            self.assertEqual(json.loads(opt(a, "--json-schema")),
                             harness.answer_schema(TASK))

    def test_a_has_no_builtin_tools(self):
        a = harness.claude_args(TASK, "A", args(), "/tmp/mcp.json")
        self.assertEqual(opt(a, "--tools"), "")
        self.assertEqual(opt(a, "--allowedTools"), "mcp__zorba")
        self.assertEqual(opt(a, "--mcp-config"), "/tmp/mcp.json")

    def test_b_has_bash_only(self):
        a = harness.claude_args(TASK, "B", args())
        self.assertEqual(opt(a, "--tools"), "Bash")
        self.assertEqual(opt(a, "--allowedTools"), "Bash")
        self.assertNotIn("--mcp-config", a)
        self.assertEqual(json.loads(opt(a, "--settings")),
                         {"bashOutputMaxChars": 50000})

    def test_env_limits(self):
        a = harness.claude_env(args(), "A")
        self.assertEqual(a["MAX_MCP_OUTPUT_TOKENS"], "50000")
        self.assertNotIn("BASH_DEFAULT_TIMEOUT_MS", a)
        b = harness.claude_env(args(), "B")
        self.assertEqual(b["BASH_DEFAULT_TIMEOUT_MS"], "30000")
        self.assertEqual(b["BASH_MAX_TIMEOUT_MS"], "30000")
        self.assertEqual(b["CLAUDE_CODE_DISABLE_BACKGROUND_TASKS"], "1")
        self.assertEqual(b["BASH_MAX_OUTPUT_LENGTH"], "50000")
        for env in (a, b):
            self.assertEqual(env["CLAUDE_CODE_MAX_OUTPUT_TOKENS"], "16000")
            self.assertEqual(env["CLAUDE_CODE_DISABLE_CLAUDE_MDS"], "1")

    def test_stub_accepts_the_command_line(self):
        for cfg in ("A", "B"):
            a = harness.claude_args(TASK, cfg, args(effort="high"),
                                    "/tmp/mcp.json")
            with mock.patch.object(sys, "argv", ["claude"] + a):
                ns = stub_claude.parse_args()
            self.assertEqual(ns.tools, "" if cfg == "A" else "Bash")

    def test_expected_tools(self):
        self.assertEqual(harness.expected_tools("A", ["run_query"]),
                         {"StructuredOutput", "mcp__zorba__run_query"})
        self.assertEqual(harness.expected_tools("B", []),
                         {"StructuredOutput", "Bash"})


class ParseStreamTest(unittest.TestCase):
    def test_events(self):
        lines = [
            {"type": "system", "subtype": "init", "tools": ["Bash"]},
            {"type": "assistant", "parent_tool_use_id": None, "message": {
                "id": "m1", "content": [
                    {"type": "text", "text": "Looking."},
                    {"type": "tool_use", "id": "u1", "name": "Bash",
                     "input": {"command": "ls"}}]}},
            {"type": "user", "parent_tool_use_id": None, "message": {
                "content": [{"type": "tool_result", "tool_use_id": "u1",
                             "content": [{"type": "text", "text": "a b"}],
                             "is_error": False}]}},
            {"type": "result", "subtype": "success", "num_turns": 2,
             "structured_output": {"answer": {"id": "x"}}},
        ]
        s = harness.parse_stream(
            [json.dumps(x) for x in lines] + ["not json", ""])
        self.assertEqual(s["init"]["tools"], ["Bash"])
        self.assertEqual([u["id"] for u in s["uses"]], ["u1"])
        self.assertEqual(s["results"]["u1"], ("a b", False))
        self.assertEqual(s["text"], "Looking.")
        self.assertEqual(s["result"]["structured_output"]["answer"],
                         {"id": "x"})


class EgressProxyTest(unittest.TestCase):
    def test_allow_and_deny(self):
        class Echo(socketserver.BaseRequestHandler):
            def handle(self):
                self.request.sendall(self.request.recv(100))

        up = socketserver.ThreadingTCPServer(("127.0.0.1", 0), Echo)
        threading.Thread(target=up.serve_forever, daemon=True).start()
        target = f"127.0.0.1:{up.server_address[1]}"
        egress_proxy.Handler.allowed = frozenset([target])
        px = egress_proxy.Server(("127.0.0.1", 0), egress_proxy.Handler)
        threading.Thread(target=px.serve_forever, daemon=True).start()
        try:
            def connect(t):
                s = socket.create_connection(px.server_address, timeout=5)
                s.sendall(f"CONNECT {t} HTTP/1.1\r\n\r\n".encode())
                return s, s.recv(100)

            s, status = connect(target)
            self.assertIn(b" 200 ", status)
            s.sendall(b"ping")
            self.assertEqual(s.recv(100), b"ping")
            s.close()
            s, status = connect("earthquake.usgs.gov:443")
            self.assertIn(b" 403 ", status)
            s.close()
        finally:
            px.shutdown()
            px.server_close()
            up.shutdown()
            up.server_close()


if __name__ == "__main__":
    unittest.main()
