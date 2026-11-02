### Added

- Add the `cast-letter-booleans` option to `csv:parse`. With `false`, the
  unquoted single letters `T`, `Y`, `F` and `N` stay strings instead of being
  cast to booleans; `true`/`false` words and numbers are still cast. The
  default is `true`, so existing calls behave as before.

### Changed

- Make `dir:documents` and `dir:open` keep the unquoted letters `T`, `Y`, `F`
  and `N` in CSV files as strings. Before, a column such as `D`/`N`
  (day/night) silently became booleans (`N` became `false`).
