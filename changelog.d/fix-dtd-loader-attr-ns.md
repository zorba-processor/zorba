### Fixed

- Fix DTD-validated parsing (`x:parse` with `DTD-validate`, `fn:doc` with the
  `dtd` feature): attributes that share a local name in different namespaces,
  such as `a:x` and `b:x`, all got the value of the first one.
