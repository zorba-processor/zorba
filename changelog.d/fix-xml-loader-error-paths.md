### Fixed

- Fix a document read with the `dtd` feature when memory runs out as the loader
  walks the tree libxml2 built: a text, CDATA or processing instruction node
  whose content could not be allocated was loaded as an empty node, with no
  error. The load now fails as it does for any other allocation failure, and
  so does a comment whose content could not be allocated.
- Fix a leak of an attribute value when memory runs out while the same loader
  builds an attribute node.
