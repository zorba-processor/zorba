### Fixed

- `x:parse` with `<opt:DTD-validate/>` reports a well-formedness or validity
  error with libxml2's message and its line and column in the parsed string,
  as `x:parse` without DTD validation does, instead of a generic
  `zerr:ZSTR0021` with no location.
