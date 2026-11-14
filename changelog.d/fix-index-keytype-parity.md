### Fixed

- The probes of an index whose key type is `xs:double` raise `err:XPTY0004`
  for a search key that is not a number or an untyped value, as the
  comparison without an index does. They accepted any search key.
- `probe-index-point-general` on a general index whose key type is
  `xs:untypedAtomic*` accepts a numeric or other search key and compares the
  untyped keys as `=` does. It raised `err:XPTY0004` for any search key other
  than `xs:string`, `xs:anyURI` or an untyped value.
- The value range probes of a general index without a key type raise
  `err:XPTY0004` for a duration bound that has no order with a key, as `ge`
  and `le` do: an `xs:yearMonthDuration` with an `xs:dayTimeDuration`, and an
  `xs:duration` with any duration.
