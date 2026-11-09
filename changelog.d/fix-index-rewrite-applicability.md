### Fixed

- Fix joins on a general comparison (`=`) that the optimizer ran through an
  index although the comparison can raise an error: a pair whose comparison
  raises FORG0001 (an untyped value whose cast to `xs:gYear`, `xs:hexBinary`,
  `xs:base64Binary`, a number or another type fails) or XPTY0004 (types that
  cannot be compared) was skipped instead of raising the error. Such a join
  now uses an index only if the static types of both operands are strings or
  untyped, both numeric, or of one atomic type with no untyped value; joins
  of operands of static type `xs:anyAtomicType`, such as attributes of nodes
  in a collection or of an external variable, run without an index.
