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
  calling `exit()`, and its per-thread setup can run again; a failure in a libxml2
  callback is kept by the loader and thrown after the parse; the QName pool undoes
  an insertion, and a hash table keeps its old table if a resize cannot allocate;
  casts and other handlers of `std::exception` let it through; a catch clause of a
  query does not catch it. The main stack takes its size before the limit
  is set, up to 64 MiB, since at the limit it could not grow. A line of input takes at
  most the size of a message.

  `exhaust_sweep_*` in `test/mcp_tests.cpp` make each allocation of a call fail in
  turn. What is left, each of them listed in those tests:
  - Whatever else leaves a `noexcept` function: status 3 and a message
    (`on_terminate()`, `--help`).
  - Xerces-C throws from a destructor releasing a schema, at the end of a query that
    imports one, as the directory and xml modules do: abort.
  - libxml2 and glibc abort if they cannot set up the first use by a thread. The main
    thread and the scanner's do that at start.
  - One allocation near the end of a `dir:open` query ended in SIGSEGV with no stack
    to read (twice in 1990, not explained): the process ends.
  - ICU crashes in `RegexPattern::zap()` (SIGSEGV) if an allocation fails while it
    compiles a regular expression. The directory module uses some on each call.
  - The flex scanner calls `exit(2)` if it cannot allocate, while a query compiles.
  - While a query compiles, the parser reads and frees parse nodes it has already
    freed after a failure (`release_hack` in `xquery_parser.cpp`, seen with ASan and
    TSan), so the sweeps that include compilation do not run in those builds. In a
    release build it ended in SIGSEGV twice (arm64): the parser released a parse
    node a second time while it destroyed the symbols on its stack after a failed
    allocation. The process ends.
  - Running out of memory while the message cannot be parsed (before its id is read,
    and again when it is read once more for its id): the error response has
    `"id": null`, as JSON-RPC 2.0 (section 5) requires when the id cannot be
    determined. A notification gets no response when it can be recognized as one,
    also when memory runs out.
  - While a query compiles, a failure can be reported as an error of the query, for
    example that a module was not found.
  - A failure in the checks of a path can be reported as a denied or invalid path, or
    in a `dir:open` as an error record, which a query can read as no content.

  The sweeps measure what a failed call keeps through `operator new` only: memory
  that libxml2 or MAPM keep through `malloc` is not counted.
  After several consecutive failed allocations (runs of 8 in the sweeps), the XML,
  fragment and HTML parsers can keep 8 to 18 KB per failed call. The cause is not
  established. `exhaust_sweep_xml`, `_fragment` and `_html` report it for runs of 8
  but do not fail on it; for single failures the check still fails.

Tests hold the threads at known points with `ServerConfig::scan_hook`. Tests
under ThreadSanitizer: the `tsan-mcp` job of `sanitizers.yml`.
