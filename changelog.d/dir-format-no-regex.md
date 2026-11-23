### Fixed

- `dir:format`, and through it `dir:open` and `dir:documents`, find the
  extension of a path without a regular expression. When an allocation failed
  while ICU compiled the expression, ICU could return one that never matches,
  with no error: the file then had no format, `dir:open` returned an
  `UNSUPPORTED_FORMAT` error record, and the MCP server's document cache marked
  the file as not to be cached until it changed.
