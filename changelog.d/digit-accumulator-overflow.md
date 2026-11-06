### Fixed

- Fix a signed integer overflow in builds without ICU when a regular
  expression quantifier bound (`{n}` or `{n,m}`) is above 2147483647. Such a
  pattern is now rejected with `err:FORX0002`, as in builds with ICU.
