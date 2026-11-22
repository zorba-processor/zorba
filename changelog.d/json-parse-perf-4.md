### Changed

- Parse-time projection of `parse-json` at `-O1` and `-O2` also follows the
  `for` variable through `group by`, where it is bound to the values of each
  group: counting them (`count`, `empty`, `exists`), predicates over them whose
  `$$` is only looked up with literal keys (`$e[$$.http.status ge 500]`), and
  lookups of literal keys in them, also in aggregates (`max($e.http.duration_ms)`)
  and written as `$v("key")`, build only the keys they name.
