### Fixed

- Revalidation after an update that leaves a simple-typed element without a
  text child (for example deleting the text of an `xs:hexBinary`,
  `xs:base64Binary` or list-typed element) no longer aborts the process. The
  element gets the typed value validation gives the same element; for
  `xs:string` elements `fn:data` returns it instead of raising FOTY0012.
