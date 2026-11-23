### Changed

- Parse-time projection of `parse-json` at `-O1` and `-O2` follows chained
  calls of literal keys as it follows the dot form: `$e("http")("status")`
  builds only `status` of `http`, as `$e.http.status` does, instead of all of
  `http`.
