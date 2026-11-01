### Fixed

- Schema validation with the patched Xerces-C fork (`v3.3.0-zorba.5`, used by the Linux CI, builder and `zorba` runtime images): an `xs:choice` with an empty `xs:sequence` branch accepts empty content and each of the other branches (XERCESC-2243), and a union type no longer collapses whitespace in values of a whitespace-preserving member type; each member applies its own `whiteSpace` facet (XERCESC-2115). Stock Xerces-C builds still have both bugs.
