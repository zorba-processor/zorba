### Fixed

- A failed allocation while the query text is scanned no longer exits the
  process with status 2: the compilation fails with `std::bad_alloc`, as other
  failed allocations do. Another failure of the scanner raises `ZXQP0003`.
- A failed allocation while the parser is set up no longer terminates the
  process: `std::bad_alloc` reaches the caller.
