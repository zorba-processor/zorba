### Fixed

- Fix an update below an element of type `xs:anyType` leaving the schema types
  of the element's ancestors in place: with `declare revalidation skip`, an
  ancestor validated against a declaration of another type kept that type for
  content it no longer matched.
