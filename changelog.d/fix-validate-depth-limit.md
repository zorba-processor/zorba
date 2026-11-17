### Fixed

- A document that loads can be validated: `validate` gave every element of
  the validated copy its own base URI, held in an attribute that lengthened
  the node ids of its children, so a document over about 680 levels deep
  failed validation with `ZSTR0030` (`XPDY0130` since the node id depth
  limit change). An element now gets a base URI only where it differs from
  its parent's.
