### Breaking changes

- `fn:parse-xml`, `fn:parse-xml-fragment`, `x:parse` and `x:parse-html` raise
  `err:XPDY0130` (XQuery 3.1, implementation-dependent limit exceeded) instead
  of `FODC0006` when the input exceeds a limit of the parser that is not
  memory: element nesting deeper than 256 levels, the entity amplification
  factor (libxml2 2.11 and later), and libxml2's other resource limits
  (libxml2 2.13 and later). The message names the limit and its value, e.g.
  `implementation limit exceeded: element nesting depth of 256`; the line and
  column in the parsed string are the error's data location. Well-formedness
  errors and running out of memory are reported as before.
