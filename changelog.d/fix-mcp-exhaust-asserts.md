### Fixed

- Fix the release of a parse node twice when memory runs out while a query
  is parsed: a handle a grammar rule had taken on a node of the parser's
  stack could free it before the parser released it again.
- Fix the loss of a parsed XML document (`fn:parse-xml`, `doc()`) when memory
  runs out as the loader finishes it: the document stayed allocated after the
  query ended.
