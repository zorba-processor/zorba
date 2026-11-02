### Changed

- Limit JSON nesting depth to 1500 levels of arrays and objects. `jn:parse-json`
  and the directory module refuse deeper documents, and array and object
  constructors, updates and copies refuse to make deeper values, with
  `zerr:ZXQP0071`; in `zorba-mcp` a file that is too deep gets its own error
  record and the other files are not affected.

### Fixed

- Fix crashes on deeply nested JSON values: serializing, `fn:deep-equal`,
  copying, function result caching, `item:size`, `jn:encode-for-roundtrip`,
  `jn:decode-from-roundtrip`, JSound and destroying a value no longer run out
  of stack; on a thread with too little stack they raise `zerr:ZXQP0071`.
