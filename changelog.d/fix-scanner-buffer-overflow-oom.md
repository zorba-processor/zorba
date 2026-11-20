### Fixed

- A failed allocation while the scanner grows its input buffer for a token
  longer than the buffer raises `std::bad_alloc` (the MCP server's
  `memory_limit`) instead of `ZXQP0003`.
