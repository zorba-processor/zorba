### Changed

- A `%an:unique` index whose `xs:string`, `xs:anyURI` or untyped key has a
  collation other than the codepoint collation, from its `collation` clause
  or from the default collation of its module, now rejects keys that the
  collation makes equal, such as `"x"` and `"X"` under a collation that ignores
  case; before, every index compared keys by codepoint and accepted them.
- The keys of an index declared `as xs:anyURI` or `as xs:untypedAtomic`, and of
  a general index without a key type, take the default collation of the module
  that declares the index, as `xs:string` keys do.

### Fixed

- Fix indexes ignoring their collation: every value and general index,
  declared or built for a join, compared its keys by codepoint, whatever the
  `collation` of its key or the default collation of the join, so under a
  collation other than the codepoint collation probes and joins missed keys
  that the collation makes equal, such as `"x"` and `"X"` under a collation
  that ignores case.
- Do not answer a value comparison with an automatic index whose key has a
  collation other than the default collation of the comparison.
- Compare `xs:anyURI` values under the collation of the comparison, as
  `xs:string` values: under a collation other than the codepoint collation,
  `eq`, `=`, `lt` and the other comparisons, `fn:distinct-values` and index
  keys compared `xs:anyURI` values by codepoint.
- Hash an `xs:anyURI` value as the equal `xs:string` value:
  `fn:distinct-values(("x", xs:anyURI("x")))` returned both, `group by` put
  them in separate groups, and a probe of an `xs:string` index with an
  `xs:anyURI` key missed the equal key.
- Fix a join on two `xs:untypedAtomic` operands under a collation other than
  the codepoint collation: it missed pairs that the collation makes equal but
  that differ by codepoint.
