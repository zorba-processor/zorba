### Fixed

- A failed allocation while the parser reads an xs:integer or xs:double
  literal is reported as out of memory instead of err:FOAR0002 (numeric
  overflow).
