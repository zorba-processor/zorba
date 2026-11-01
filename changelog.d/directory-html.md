### Changed

- The directory module (`dir:open`, `dir:documents`, `zorba --directory`,
  `zorba-mcp`) parses `.html` and `.htm` files with `x:parse-html` and returns
  the document node as `content`; a file that cannot be parsed yields the
  per-file error record.
