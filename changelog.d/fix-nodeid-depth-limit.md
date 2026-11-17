### Breaking changes

- A tree deeper than node ids allow (ids are at most 255 bytes, about a
  thousand levels for a chain of single children) raises `err:XPDY0130`
  (implementation limit exceeded). Loading such a document with `fn:parse-xml`,
  `fn:parse-xml-fragment` or `x:parse` without DTD validation raised
  `FODC0006` naming the byte limit; the message now names the depth of the
  node that cannot get an id, e.g. `implementation limit exceeded: node
  nesting depth of 1019 exceeds the node id limit`, with the line and column
  in the input as the data location (also for `fn:parse-xml-fragment`).
  Building nodes whose ids exceed the limit while constructing, copying or
  updating raised `ZSTR0030`; it now raises the same error, naming the depth
  counted from the root of the tree.
