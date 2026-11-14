### Fixed

- A parser error of `fn:parse-xml-fragment` and of `x:parse` with
  `parse-external-parsed-entity` is reported at its line and column in the
  parsed string: the column on the first line no longer counts the text the
  parser adds before the fragment. An element the string leaves open is
  reported as for a document (`premature end of content for tag starting on
  line N`, at the end of the string), and an end tag without a start tag as
  `XML chunk is not well-balanced`; neither message names an element the string
  does not have.
