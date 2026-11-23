### Fixed

- After revalidation (`declare revalidation`), the typed value of an element
  of a complex type with simple content is a value of the simple type the
  complex type is derived from (`xs:integer` for an extension of
  `xs:integer`), as validation gives it. It was `xs:untypedAtomic`, though
  the element's type annotation was right.
