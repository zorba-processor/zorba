### Fixed

- The children of an element without a declaration whose `xsi:type` names a
  complex type are validated against the content model of that type in
  `validate lax` and `validate strict`: they get their declared types, and a
  child the content model does not allow or a missing required child raises
  XQDY0027. Before, the content model was checked as if the element had no
  children, so valid content raised XQDY0027.
