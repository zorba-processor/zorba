### Changed

- At `-O1` and `-O2`, `parse-json` builds of each value only what the query
  reads when a `for` clause iterates over the parse and the rest of the FLWOR
  uses its variable only in lookups of literal keys (`$v.a.b`), or when the
  values are only counted: the rest is checked without being built, so
  syntax errors, duplicate keys and the nesting limit are still reported, with
  the same locations as at `-O0`.
