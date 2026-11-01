### Fixed

- Fix JSON objects keeping a pointer into a temporary copy of each key: when a
  key string was not shared with its copy (as with the values `csv:parse`
  returned after an unquoted `true` or `false`), the object read freed memory
  on every later lookup, which could drop or garble object fields or crash.
  Objects now own their key strings.
