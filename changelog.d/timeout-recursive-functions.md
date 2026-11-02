### Fixed

- Stop recursive functions at the time limit (`--timeout`, `zerr:ZXQP0068`):
  the deadline count restarted with each function call, so a recursion that did
  little per call was never checked and ran until it ran out of memory. Function
  calls and external functions now read the clock, and the check runs every 64
  iterator calls instead of 1024.
- Release the memory of a completed function call: a recursion kept about 11 KB
  per call made until the query ended, so peak memory doubled with each level of
  a tree recursion. Peak memory now follows the depth of the recursion. A call
  abandoned by `head()` or a positional filter is released with its caller.
- Count string operations and sort comparisons against the time limit: a query
  doing long string operations ran for tens of seconds past the limit.
- Stop constant folding from using up the time limit: the optimizer ran each
  constant expression under the time limit and tried each enclosing expression
  first, so a query ran for the limit once per level of nesting before it
  started. Folding now has a budget of 500 ms per query.
- Stop leaving an expression behind for each dynamic function call, about 400
  bytes until the query ended.

### Added

- Add the `--max-memory` option to `zorba-mcp` (default 4 GiB; Linux only): a
  query that needs more ends with a tool error with code `memory_limit` and the
  server keeps answering, instead of being killed by the system.
