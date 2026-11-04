### Changed

- Error messages shorten the data they quote. A value over 100 bytes becomes its
  head, an ellipsis, its tail and its size in bytes, as in
  `"abcdefghij…abcdefghij (3000000 bytes)": value of type xs:string is not
  castable to type xs:integer`; `$err:description` in a `catch` clause holds the
  same text. The rest of the message stays complete. A URI, a path or a QName
  stays whole up to 4096 bytes, and so does the text of an error, such as the
  message of the parser, of a library or of a nested error, or the name of a
  type. The description given to `fn:error` is not shortened.
- `zorba-mcp`: `--max-output` also limits the `message` of an error: a longer
  one loses its middle and keeps its end, with a marker that states its total
  size. A hint is at most 4 KB, and `location.text` shows about 512 bytes around
  the column of the error when the line is longer than that, or the two ends of
  a line over 65535 bytes, where a column cannot be trusted. A selector over 100
  bytes is not repeated in the hint for a string selector on an array.
