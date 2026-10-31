### Fixed

- xs:decimal arithmetic (and xs:integer arithmetic in `ZORBA_WITH_BIG_INTEGER`
  builds) is now thread-safe without a global lock: MAPM's internal work
  buffers, caches and constants are per-thread, and the reference count of
  shared MAPM values is atomic. Concurrent queries in one process no longer
  serialize on, or race in, the arbitrary-precision library.

### Added

- `test/libunit/mapm_threads`, a multithreaded decimal/integer stress test, and
  a ThreadSanitizer job (`tsan`, `tsan-bigint` presets) in the Sanitizers
  workflow.
- `j8_decimal` benchmark query (decimal arithmetic).
