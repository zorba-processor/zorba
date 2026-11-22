### Changed

- Make `parse-json` faster again: the JSON lexer reads the stream in blocks
  and scans them with a pointer, strings without escapes and numbers are not
  copied before they become items, line and column come from offsets instead
  of being counted per character, and the parser passes tokens on without
  copying them. On a synthetic 213 MB NDJSON log, counting the events takes
  2.4 s instead of 3.8 s, with 128 allocations per event instead of 224.
