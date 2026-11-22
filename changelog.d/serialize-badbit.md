### Fixed

- Running out of memory while `fn:serialize`, `jn:encode-for-roundtrip` of a
  node, `jx:xml-to-json` of a string with markup, `zq:query-plan` or a
  zorba-query URL resolver write their result into a string stream is now the
  memory error. It was `zerr:ZOSE0004` (bad I/O stream state), which
  `try`/`catch *` caught. `random:uuid`, the query ids of
  `zq:prepare-main-module` and `zq:load-from-query-plan`, Roman numerals of
  `format-integer` and of the date and time picture functions, and their
  calendar and language markers are no longer cut short then.
