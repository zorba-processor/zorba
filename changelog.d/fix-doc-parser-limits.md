### Breaking changes

- `fn:doc` on a document that exceeds a parser limit (element nesting depth
  of 256 with DTD validation, entity amplification, node id depth) raises
  `err:XPDY0130` (implementation limit exceeded), as `fn:parse-xml` and
  `x:parse` do, instead of `FODC0002`. The message names the limit; the data
  location is the document URI, line and column. `FODC0002` stays for
  documents that are missing, unreadable or not well-formed.

### Fixed

- The data location of an entity amplification error is where the document
  references the entity, with the document's URI; it was a position in the
  entity's replacement text, without the document.
