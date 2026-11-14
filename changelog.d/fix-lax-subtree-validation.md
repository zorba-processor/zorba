### Fixed

- A declared element below an element without a declaration is strictly
  assessed in `validate lax` and `validate strict`: it gets its type
  annotation, and an invalid one raises XQDY0027. Before, the whole subtree
  of an element without a declaration was left untyped and unchecked. Below a
  `processContents="skip"` wildcard nothing is assessed, as before.
- An element without a declaration that has an `xsi:type` and
  `xsi:nil="true"` is validated against that type and is not nilled. Before,
  `validate strict` rejected it because no nillable declaration allowed
  `xsi:nil`.
- An `xsi:type` followed by another attribute on the same element, such as
  `xsi:nil`, is applied. Before, the value of the next attribute replaced the
  `xsi:type` value, so validation reported a missing grammar or ignored the
  type.
