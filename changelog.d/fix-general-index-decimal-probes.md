### Fixed

- Fix general index probes with an `xs:decimal` or `xs:integer` value, which
  compared it with `xs:decimal` keys through their `xs:double` images instead
  of exactly, and so returned keys that differ from the value only beyond
  `xs:double` precision (for example key `0.1000000000000000000001` for the
  value `0.1`); a probe could also crash on a key of that kind, and an integer
  key above 99999999999 could be returned twice.
