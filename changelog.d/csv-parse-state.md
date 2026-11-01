### Fixed

- Fix `csv:parse` reusing the field names and options of a previous input when
  the same call parses several inputs (in a FLWOR expression or a function
  called more than once): a second input with a different header raised
  `csv:EXTRA_VALUE` or returned its header line as a record with the first
  input's field names, and `dir:documents` gave every CSV file after the first
  the first file's field names.
- Fix `csv:parse` returning values after an unquoted `true` or `false` whose
  use as object keys read freed memory, which could drop object fields or
  crash.
