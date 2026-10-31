### Fixed

- Schema validation with the patched Xerces-C fork (`v3.3.0-zorba.4`, used by the Linux CI, builder and `zorba` runtime images): `NaN` no longer passes `minInclusive`/`maxInclusive` facets on `xs:double`/`xs:float` (XERCESC-2263), and `length`, `minLength` and `maxLength` count characters instead of UTF-16 code units, so a character outside the Basic Multilingual Plane counts as one (XERCESC-2063). Stock Xerces-C builds still have both bugs.
