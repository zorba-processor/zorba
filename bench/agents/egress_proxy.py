#!/usr/bin/env python3
"""Allowlisting HTTPS proxy for configuration B of the claude-code backend.

Claude Code runs inside the sandbox container, on a Docker network without a
route to the outside. This proxy is the container's only way out: it accepts
HTTP CONNECT requests for the allowed host:port pairs (by default the
Anthropic API) and refuses everything else, so neither Claude Code nor the
commands its Bash tool runs can reach any other host (for example the live
feeds the snapshot was taken from). Plain HTTP requests are refused too.

Each decision is logged to standard error ("allow host:port" or
"deny host:port").

Usage: egress_proxy.py [--port 3128] [--allow host:port ...]
"""

import argparse
import select
import socket
import socketserver
import sys


class Handler(socketserver.BaseRequestHandler):
    allowed = frozenset()

    def handle(self):
        conn = self.request
        conn.settimeout(30)
        head = b""
        try:
            while b"\r\n\r\n" not in head and len(head) < 16384:
                chunk = conn.recv(4096)
                if not chunk:
                    return
                head += chunk
        except OSError:
            return
        line = head.split(b"\r\n", 1)[0].decode("latin-1", "replace")
        parts = line.split()
        target = parts[1] if len(parts) == 3 else "?"
        if len(parts) != 3 or parts[0].upper() != "CONNECT" \
                or target.lower() not in self.allowed:
            print(f"deny {line[:200]}", file=sys.stderr, flush=True)
            conn.sendall(b"HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n"
                         b"Connection: close\r\n\r\n")
            return
        host, _, port = target.rpartition(":")
        try:
            upstream = socket.create_connection((host, int(port)), timeout=30)
        except OSError as e:
            print(f"fail {target}: {e}", file=sys.stderr, flush=True)
            conn.sendall(b"HTTP/1.1 502 Bad Gateway\r\nContent-Length: 0\r\n"
                         b"Connection: close\r\n\r\n")
            return
        print(f"allow {target}", file=sys.stderr, flush=True)
        conn.sendall(b"HTTP/1.1 200 Connection established\r\n\r\n")
        rest = head.split(b"\r\n\r\n", 1)[1]
        if rest:
            upstream.sendall(rest)
        conn.settimeout(None)
        upstream.settimeout(None)
        socks = [conn, upstream]
        try:
            while True:
                ready, _, _ = select.select(socks, [], [], 600)
                if not ready:
                    return
                for s in ready:
                    data = s.recv(65536)
                    if not data:
                        return
                    (upstream if s is conn else conn).sendall(data)
        except OSError:
            return
        finally:
            upstream.close()


class Server(socketserver.ThreadingTCPServer):
    daemon_threads = True
    allow_reuse_address = True


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--port", type=int, default=3128)
    ap.add_argument("--allow", action="append", default=[],
                    help="host:port to allow (repeatable; default "
                    "api.anthropic.com:443)")
    args = ap.parse_args()
    Handler.allowed = frozenset(
        a.lower() for a in (args.allow or ["api.anthropic.com:443"]))
    print(f"egress proxy on :{args.port}, allowing "
          f"{', '.join(sorted(Handler.allowed))}", file=sys.stderr, flush=True)
    with Server(("0.0.0.0", args.port), Handler) as srv:
        srv.serve_forever()


if __name__ == "__main__":
    main()
