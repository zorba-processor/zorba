### Breaking changes

- A tree deeper than node ids allow (ids are at most 255 bytes, about a
  thousand levels for a chain of single children) raises `err:XPDY0130`
  (implementation limit exceeded). Loading such a document with `fn:parse-xml`,
  `fn:parse-xml-fragment` or `x:parse` without DTD validation raised
  `FODC0006` naming the byte limit; the message now names the
  element nesting depth reached, e.g. `implementation limit exceeded: element
  nesting depth of 1018 exceeds the node id limit`, with the line and column in
  the input as the data location. Constructing or copying such a tree raised
  `ZSTR0030`; the message now names the depth of the node that cannot get an
  id, counted from the root of the new tree.
