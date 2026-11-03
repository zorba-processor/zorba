### Fixed

- `fn:distinct-values` and `group by` now merge an `xs:integer` or `xs:decimal`
  with the `xs:float` it equals after promotion: `16777217 eq xs:float(16777216)`
  is true and they are one value. A container that sees an `xs:float` key hashes
  all its numeric keys on their `xs:float` rounding from then on, so large
  integers share buckets only there; a container without `xs:float` keys costs
  the same as before.
- A join on operands of static type `xs:anyAtomicType` no longer loses pairs with an
  `xs:float` on one side. When an `xs:float` takes part it runs as a nested loop.
- `map:create` accepts the key type `"anyAtomicType"`; such a map no longer raises
  `ZSTR0040` when keys of different types meet.
- Comparing numeric items of different types directly (index and map keys, some
  `group by` keys) goes by value: `5` no longer equals `5.5e0`, and both
  directions agree.
- `xs:unsignedLong` holds its full range without big integers: casts from strings,
  numbers and `xs:untypedAtomic` up to 18446744073709551615, comparisons, hashing,
  promotion to `xs:decimal`, `xs:double` and `xs:float`, and serialization.
  `xs:unsignedLong` values above 2^63-1 used in `xs:integer` arithmetic still
  raise `FOAR0002` in that build.
- Fix a signed overflow in the conversion of the largest `long long` to a string.
