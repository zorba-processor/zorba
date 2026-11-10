### Fixed

- Fix indexes ignoring their collation: every value and general index,
  declared or built for a join, compared its keys by codepoint, whatever the
  `collation` of its key or the default collation of the join, so under a
  collation other than the codepoint collation probes and joins missed keys
  that the collation makes equal, such as `"x"` and `"X"` under a collation
  that ignores case.
- Compare `xs:anyURI` values under the collation of the comparison, as
  `xs:string` values: under a collation other than the codepoint collation,
  `eq`, `=`, `lt` and the other comparisons, `fn:distinct-values` and index
  keys compared `xs:anyURI` values by codepoint.
- Fix a join on two `xs:untypedAtomic` operands under a collation other than
  the codepoint collation: it missed pairs that the collation makes equal but
  that differ by codepoint.
