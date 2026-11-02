### Fixed

- Compare `xs:integer` with `xs:decimal` by value in `group by`, `order by`,
  `fn:index-of`, joins, typed value comparisons and `unordered-maps` keys:
  the decimal was converted to an integer first, which truncated it (so `1` and
  `1.5` compared equal) or raised an internal error beyond 2^63.
- Fix an internal error when comparing a negative integer with an
  `xs:nonNegativeInteger`.
- Give `xs:long` and `xs:unsignedLong` values above 2^53 the same hash as an
  equal `xs:integer`, `xs:decimal` or `xs:double`: `fn:distinct-values` and
  `group by` kept them as separate values.
- Order `order by` keys that mix numeric types in their least common type:
  each pair was promoted on its own, which gave no consistent order around the
  xs:double rounding limits.
