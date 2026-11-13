### Fixed

- The allocation-failure sweeps of the MCP server tests no longer allow a crash
  in glibc's `pthread_getattr_np()`: the test allocator now sets `errno` to
  `ENOMEM` when it makes an allocation fail, as glibc's `malloc` does and glibc
  itself relies on.
