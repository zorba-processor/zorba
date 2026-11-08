### Fixed

- Fix DTD-validated parsing (`x:parse` with `DTD-validate`, `fn:doc` with the
  `dtd` feature) giving attributes that share a local name in different
  namespaces, such as `a:x` and `b:x`, all the value of the first one.
