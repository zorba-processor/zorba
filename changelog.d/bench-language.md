### Added

- The agent benchmark (`bench/agents/`) can choose the query language of the
  zorba-mcp configuration: `harness.py --language xquery|jsoniq` (default
  `xquery`) is passed to `zorba-mcp --language` and recorded in the results,
  and the `Agent benchmark` workflow has a matching `language` input.
