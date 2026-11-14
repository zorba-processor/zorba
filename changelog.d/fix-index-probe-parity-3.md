### Fixed

- General probes of an index holding keys of one class (strings, numbers,
  booleans, dates, durations, ...) raise `err:XPTY0004` for a search key of
  another class, as `=`, `>=` and `<=` do. They returned no match. Untyped
  keys are cast to the type of the search key, as before.
- Range probes of a value range index raise `err:XPTY0004` for a bound that
  `ge` and `le` cannot compare with a key, such as any duration bound in an
  index whose key type is `xs:duration`. They returned the keys in range.
