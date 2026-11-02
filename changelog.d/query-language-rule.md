### Added

- Add the `--language xquery|jsoniq` option to `zorba` and `zorba-mcp`: the
  language of a query that has no `jsoniq version` or `xquery version`
  declaration and is not read from a `.jq` or `.xq` file. The default is
  `xquery`. `zorba-mcp` names the language in the description of `run_query`.

### Changed

- A query is compiled in the language of its own `jsoniq version` or
  `xquery version` declaration, else of its file extension (`.jq` is JSONiq,
  `.xq` is XQuery), else of `--language`. `zorba -f query.xq --jsoniq` now
  compiles `query.xq` as XQuery. The `.jq` files of `zorba-mcp --queries` are
  compiled as JSONiq (they were compiled as XQuery unless they declared
  JSONiq).

### Fixed

- Fix `xquery version` in a query compiled with the JSONiq static context
  (`zorba --jsoniq`): the query now follows XQuery name resolution instead of
  JSONiq's.
