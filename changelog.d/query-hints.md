### Changed

- `zorba-mcp`: the `run_query` description carries a short JSONiq primer: the
  context item is `$$`, the `content` of a CSV or of a top-level JSON array is
  an array, `-` belongs to names, `\` escapes in strings, `math:` needs no
  import, there is no type `number`, and a window clause. Its examples run in
  the test suite. The server instructions point to it.
- Error messages gain hints: `.` in JSONiq points to `$$`; `\` in a JSONiq
  string that does not start an escape; a module import whose network fetch
  the security policy denied says that the URI may hold no module, and a math
  namespace points to `http://www.w3.org/2005/xpath-functions/math`; a function
  of that namespace called in another math namespace points to it; `$a- $b`
  and undeclared names that end in `-` explain that `-` belongs to names. The
  hint for a string selector on an array also offers `$x[]` in JSONiq. Error
  codes and messages do not change.

### Fixed

- Fix diagnostic message substitution, which left `$2` literal when it came
  after a `${...3...}`.
