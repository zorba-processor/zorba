### Fixed

- `fn:parse-xml-fragment` and `x:parse` of an external parsed entity: an end
  tag `</zorba-fragment-wrapper>` at the top level of the input is an end tag
  without a start tag, reported as `XML chunk is not well-balanced` at the end
  of that tag, like any other. It closed the element the parser wraps the
  input in, and the error was "extra content at end of XML document" where
  the input ends.
- An element the input leaves open is reported where the input ends, counted
  as the parser counts the rest of the line: in the characters of the
  declared encoding. Under an encoding other than UTF-8, the column counted
  UTF-8 characters, and an error near the end of the last line could be moved
  to the end of the input.
