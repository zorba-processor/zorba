### Changed

- `zorba-mcp --language jsoniq`: the `run_query` description carries a short
  JSONiq primer: the context item is `$$`, the `content` of a CSV or of a
  top-level JSON array is an array, `-` belongs to names, `\` escapes in strings,
  `math:` needs no import, there is no type `number`, and a window clause. Its
  examples run in the test suite. The server instructions point to it. A session
  in XQuery, the default, has neither.
- Errors gain hints, and existing error codes and the message of a query error do
  not change: `.` in JSONiq, where an expression can start, points to `$$`; `\` in
  a JSONiq string that does not start an escape; a module import whose network
  fetch the security policy denied says that the URI may hold no module; a
  misspelled Zorba math namespace, denied or not, points to
  `http://zorba.io/modules/math`; a function of
  `http://www.w3.org/2005/xpath-functions/math` called in the Zorba math namespace
  points to it, unless that namespace declares the function with another arity;
  `$a- $b` and undeclared names that end in `-` explain that `-` belongs to names.
  The hint for a string selector on an array also offers `$x[]` in JSONiq, and
  does not repeat a selector computed at run time.

### Fixed

- Fix diagnostic message substitution, which left a `$n` literal when it came
  after a `${...m...}` with a higher `m`. The message of the libxml2 error
  `XML_DTD_ID_SUBSET` changes: its second name was printed as `$6`.
- Fix the message of an undefined namespace prefix in XML, which printed its
  template (`${ for "7?6"}${ on "7?7:6"}`) instead of the names.
