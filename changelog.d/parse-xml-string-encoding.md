### Fixed

- `fn:parse-xml`, `fn:parse-xml-fragment` and `parse-xml:parse` ignore the
  encoding that the XML or text declaration of their string input names, as
  F&O 3.1 requires: the string is already decoded. An input declaring
  `encoding="ISO-8859-1"` turned `é` into `Ã©`. Error locations in such input
  are now counted in the characters of the string.
