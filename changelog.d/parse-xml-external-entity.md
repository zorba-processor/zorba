### Fixed

- Fix `fn:parse-xml`, `fn:parse-xml-fragment` and the xml module's
  parse-external-parsed-entity option resolving a relative system identifier
  of an external entity against the process's working directory instead of
  the static base URI (or the `base-uri` option): an entity that the security
  policy allows is now expanded instead of silently dropped, and a refused one
  raises `zerr:ZXQP0029` naming the resolved URI (W3C QT3 test case
  `fn-parse-xml/parse-xml-010`).
