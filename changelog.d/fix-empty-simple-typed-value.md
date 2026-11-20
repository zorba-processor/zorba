### Fixed

- An element of a simple type with no text child (empty content) has the
  typed value of "" for its type, after validation and after revalidation:
  `xs:hexBinary("")`, `xs:string("")`, the user-defined atomic type's value of
  "", and the empty sequence for a list type. It was `xs:untypedAtomic("")`.
