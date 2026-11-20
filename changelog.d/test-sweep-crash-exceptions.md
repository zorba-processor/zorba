### Changed

- `zorba-mcp --help` and `bin/mcp/README.md` list as the ways a library ends the
  server when memory runs out only Xerces-C releasing a schema (abort) and ICU
  compiling a regular expression (SIGSEGV). The allocation-failure sweeps of the
  MCP server tests allow only those crashes.
