### Fixed

- `validate lax` raises XQDY0027 for an invalid element that has a
  declaration: an `xsi:type` that does not derive from the declared type, an
  abstract element, `xsi:nil` on a non-nillable declaration, a nilled element
  with content, or an identity constraint violation. Before, lax validation
  accepted these; it still skips elements without a declaration.
- An element declared without a type whose `xsi:type` names a simple type
  is invalid with element children, in strict and lax validation. Before, it
  was accepted and annotated `xs:anyType`, because the check of a child reset
  the simple type of its parent.
