### Fixed

- Fix general range index probes with an `xs:double` or `xs:float` bound,
  which returned no entry whose key was an integer.
- Fix general index probes with an `xs:untypedAtomic` value in decimal
  notation, which compared it with integer keys as an `xs:decimal` instead of
  casting it to `xs:double`, and so missed keys equal to the cast value (for
  example key 2 for the value `"2.00000000000000000001"`).
