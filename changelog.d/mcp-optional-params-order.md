### Fixed

- Fix the order of the external variables reported by `getExternalVariables`
  (C++ API) and of the tool parameters in `zorba-mcp`'s `tools/list`: they
  follow declaration order instead of an order that varied between runs.
