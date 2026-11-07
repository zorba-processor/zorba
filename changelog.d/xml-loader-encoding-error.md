### Fixed

- Fix `fn:parse-xml` and `fn:parse-xml-fragment` reporting an internal
  assertion when the input does not convert from its declared encoding and
  libxml2 (2.9) stops the parser without an error: the parse is now reported
  as a malformed document (`err:FODC0006`).
