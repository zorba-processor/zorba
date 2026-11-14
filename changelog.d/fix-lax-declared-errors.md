### Fixed

- `validate lax` raises XQDY0027 for an invalid element that has a
  declaration: an invalid simple value, an `xsi:type` that does not derive
  from the declared type, an abstract element, `xsi:nil` on a non-nillable
  declaration, or an identity constraint violation. Before, lax validation
  accepted these and only skipped the checks; it still skips elements without
  a declaration.
