### Fixed

- Fix joins on a general comparison (`=`) that the optimizer ran through an
  index although the comparison can raise an error: a pair whose comparison
  raises FORG0001 (an untyped value whose cast to `xs:gYear`, `xs:hexBinary`,
  `xs:base64Binary`, a number or another type fails) or XPTY0004 (types that
  cannot be compared) was skipped instead of raising the error. Such joins now
  run without an index unless the static types of both operands are strings or
  untyped, both numeric, or of one atomic type with no untyped value; joins of
  operands whose static type is `xs:anyAtomicType`, as for nodes not known to
  be untyped, no longer use an index.
