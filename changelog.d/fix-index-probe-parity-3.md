### Fixed

- General probes of an index holding keys of one class (strings, numbers,
  booleans, dates, durations, ...) raise `err:XPTY0004` for a search key of
  another class, as `=`, `>=` and `<=` do. They returned no match. Untyped
  keys are cast to the type of the search key, as before.
