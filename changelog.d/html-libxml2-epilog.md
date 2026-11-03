### Fixed

- HTML with libxml2 before 2.14 (Ubuntu 24.04 has 2.9.14): `x:parse-html` and `dir:open` reported an error for a document with 1000 or more blanks after `</html>`, or that ended, unclosed, on a run of text or blanks of a length that is a multiple of 1000. libxml2 could report the end of the document with elements open or not at all. These documents now parse, on every libxml2 version.
- `x:parse-html` reports a failed internal check as what it is, not as invalid content in FODC0006, and its message for a document left with open elements no longer holds a generated document URI.
