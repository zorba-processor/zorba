# zorba-mcp

An MCP server (JSON-RPC over stdio) over the Zorba C++ API. User documentation:
`site/mcp.html`; options: `zorba-mcp --help`.

| File | What it is |
|---|---|
| `zorba_mcp.cpp` | `main`: arguments, the stdio loop |
| `mcp_server.*` | the protocol and the tools; the only code that uses the engine |
| `json.*` | a small JSON value and parser, without engine types |
| `scanner.*` | the background scan behind `list_documents` |
| `scan_preview.*` | previews of CSV, JSON, XML and HTML files |
| `test/` | `mcp_tests.cpp`, and `scan_tests.cpp` for the scanner |

## The background scan

At start, `Scanner` starts two threads: the walker lists directories and the
sampler reads the head of each file. `Scanner::answer()`, on the main thread,
answers `list_documents` from what they have, plus a status. `--no-scan` turns
this off; `list_documents` then runs `dir:list` as a query.

Rules this code depends on:

- `scanner.*` and `scan_preview.*` use nothing of the Zorba engine, which is
  single-threaded (`doc/zorba/threading.dox`). Only the main thread writes to
  stdout and runs queries.
- libxml2's global state is initialized once, on the main thread, in the
  `Scanner` constructor. Each parse has its own context. `Zorba::shutdown()`
  cleans up libxml2, so `Server` stops the scanner first. Zorba changes the
  process-wide external entity loader; the scanner never reaches it, because
  it parses without loading anything.
- Files are opened from a descriptor of the root, one path component at a
  time with `O_NOFOLLOW`, because a check of a path followed by an open could
  be raced. `O_NONBLOCK` keeps a FIFO from blocking the open, and only regular
  files are read.
- Parsers get at most `ScanLimits::max_file_bytes` and report what they cannot
  parse in the preview's `error`. Nesting is bounded: 64 levels for JSON,
  256 for XML, 200 for the walk of the HTML tree, which takes no stack; deeper
  is an `error`.
- XML parser options must not gain `NOENT`, `DTDLOAD`, `DTDATTR`, `DTDVALID`,
  `XINCLUDE` or `HUGE`.
- `Scanner::stop()` waits one second. A thread still in a `read()` is detached;
  it holds the shared state, and after the read it sees the stop flag and ends
  without touching libxml2.

- Memory: everything the scanner holds is charged against
  `ScanLimits::max_memory` (`--scan-cache-mb`), on the high side, under the one
  mutex shared with the threads. The sampler stops at the limit and never evicts:
  a full pass would drop what requests asked for. A request evicts least recently
  used previews to keep what it read, and requests move a preview to the recent
  end; the scanner's previews enter at the cold end. The walker evicts previews
  before giving up its index. LRU rather than frequency counts: it is simple, and
  there is no data favoring anything else.

Tests hold the threads at known points with `ServerConfig::scan_hook`. Tests
under ThreadSanitizer: the `tsan-mcp` job of `sanitizers.yml`.
