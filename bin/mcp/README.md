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
- The request thread never parses. A file with no preview goes to the sampler
  ahead of its queue, and the request waits up to a bound from `--timeout`, then
  answers `"preview_state":"pending"`.
- Directories are opened from a descriptor of the root, one path component at a
  time with `O_NOFOLLOW`, because a check of a path followed by an open could
  be raced. A file is opened only after `fstatat` says it is regular, and
  `st_dev`/`st_ino` are compared after the open, so a FIFO or device is never
  opened.
- Markup goes through `markup_refusal()` before libxml2: time in start tag
  parsing grows with the square of one tag's attributes and cannot be
  interrupted, so a tag over 16 KB (without quoted values) or 256 attributes is
  refused. A push parser would not help: it only regains control between
  chunks. HTML is parsed with SAX in 64 KB chunks and builds no tree; there is
  no HTML reader in libxml2. Parsing stops after 5 s.
- Parsers get at most `ScanLimits::max_file_bytes` and report what they cannot
  parse in the preview's `error`. Nesting is bounded: 64 levels for JSON,
  256 for XML, 200 for the walk of the HTML tree, which takes no stack; deeper
  is an `error`.
- XML parser options must not gain `NOENT`, `DTDLOAD`, `DTDATTR`, `DTDVALID`,
  `XINCLUDE` or `HUGE`.
- `Scanner::stop()` waits one second and says whether it joined the threads. If
  not, `main` ends the process with `_Exit`: `Zorba::shutdown()` calls
  `xmlCleanupParser`, which must not run with a thread in libxml2.
- Listings go through directories in batches of 256 and hold no whole directory.
  A listing that saw a whole directory removes what it did not see, by stamps.
- What is not bounded by `--scan-cache-mb`: the buffer of the file being read (1
  MiB), libxml2's working memory for it, two thread stacks, and up to `limit`
  entries in an answer.

- Memory: everything the scanner holds is charged against
  `ScanLimits::max_memory` (`--scan-cache-mb`), on the high side, under the one
  mutex shared with the threads. The sampler stops at the limit and never evicts:
  a full pass would drop what requests asked for. A request evicts least recently
  used previews to keep what it read, and requests move a preview to the recent
  end; the scanner's previews enter at the cold end. The walker evicts previews
  before giving up its index. LRU rather than frequency counts: it is simple, and
  there is no data favoring anything else.

- `--max-memory` (`RLIMIT_AS`) is applied before the threads start. If they
  cannot start, `Server` works as with `--no-scan`. In the threads a failed
  allocation ends the scan as at `memory_limit`; it must not leave a libxml2
  callback, which is C code.

- On the main thread a failed allocation ends the query: std::bad_alloc reaches
  `Server::run()`, and is the tool error `memory_limit`. What makes this hold:
  destructors and what they call do not allocate (`JSONItem::destroy()`), or may
  throw and do not while unwinding (`string_appender`); MAPM throws instead of
  calling `exit()`; a failure in a libxml2 callback is kept by the loader and
  thrown after the parse; a catch clause of a query does not catch it. The main
  stack takes its size before the limit is set, since at the limit it could not
  grow. Whatever still leaves a `noexcept` function ends the process with status 3
  and a message (`on_terminate()`, `--help`).

Tests hold the threads at known points with `ServerConfig::scan_hook`. Tests
under ThreadSanitizer: the `tsan-mcp` job of `sanitizers.yml`.
