### Fixed

- Fix joins on a value comparison (`eq`) that the optimizer ran through an
  index although the comparison can raise XPTY0004: a pair of types that `eq`
  cannot compare, such as an untyped value and an `xs:integer`, an `xs:gYear`
  and an `xs:string`, or a JSON string and a JSON number, was skipped instead
  of raising the error. Such a join now uses an index only if the static types
  of both operands are strings, untyped or `xs:anyURI`, both numeric, or
  atomic with no untyped value and one a subtype of the other; joins of
  operands of static type `xs:anyAtomicType`, such as attributes of nodes
  from `fn:doc` or JSON values, run without an index.
