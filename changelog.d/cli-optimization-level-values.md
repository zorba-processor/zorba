### Fixed

- `zorba --optimization-level` and `-O` accept 0, 1 and 2, also written O0, O1
  and O2, and reject any other value with an error and exit status 1. Before,
  a value that did not start with a digit, such as `O1` or `O2`, compiled the
  query at level 0 without an error, and a digit followed by other characters,
  such as `1x`, was read as that digit.
