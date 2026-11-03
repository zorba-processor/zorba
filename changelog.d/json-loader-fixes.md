### Fixed

- Fix a UTF-8 byte order mark at the start of a file coming back when the
  string of `file:read-text` was read: `jn:parse-json` raised `JNDY0021`,
  `csv:parse` and `dir:open` saw it in the first field, and the string value
  started with U+FEFF. `fn:unparsed-text`, `fn:unparsed-text-lines`,
  `file:read-text-lines` and `fetch:content` now also skip the mark.
- `jn:parse-json` ignores a byte order mark at the start of its input, as
  `fn:parse-json` does in F&O 3.1.
