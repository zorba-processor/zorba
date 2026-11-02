### Fixed

- Make `zorba-mcp` reject tool arguments that the tool's input schema does not declare (for example a `variables` argument to `run_query`) with an `invalid-arguments` tool error naming them, instead of ignoring them and returning a result computed without them.
- Make `zorba-mcp` answer `ping` with "method not found" to requests of protocol revision 2026-07-28, which removed it; `ping` still works in sessions opened with `initialize`.
