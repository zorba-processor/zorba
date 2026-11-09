### Fixed

- Fix `ref:reference` declaring `xs:string` as its result type while it
  returns an `xs:anyURI`, as its documentation says. At O1 and O2 an
  expression such as `ref:reference($n) instance of xs:string` was folded to
  `true` from the declared type, while O0 gave `false`. The declared result
  type is now `xs:anyURI`, so every optimization level gives the same result.
  An `xs:anyURI` still converts to the `xs:string` parameter of
  `ref:dereference` and `ref:assign-reference`.
