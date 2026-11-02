### Fixed

- Raise an error when a parsed document refers to an external parsed entity
  that cannot be loaded (e.g. its file does not exist): `fn:doc` raises
  FODC0002, `fn:parse-xml` and `x:parse` raise FODC0006. Before, the entity
  was silently left out of the document, which XDM does not allow (all
  external parsed entities must be expanded).
