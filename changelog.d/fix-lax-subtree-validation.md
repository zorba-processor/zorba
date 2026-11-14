### Fixed

- A declared element below an element without a declaration is strictly
  assessed in `validate lax` and `validate strict`: it gets its type
  annotation, and an invalid one raises XQDY0027. Before, the whole subtree
  of an element without a declaration was left untyped and unchecked. Below a
  `processContents="skip"` wildcard nothing is assessed, as before.
