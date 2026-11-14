### Changed

- The `FODC0006` error of `fn:parse-xml`, `fn:parse-xml-fragment`, `x:parse`
  and `x:parse-html` names the code of the parser error it reports, before that
  error's message: `invalid content passed to fn:parse-xml(): [zerr:ZSTR0021]:
  ...`. The line and column in the parsed string stay in the error's data
  location.
