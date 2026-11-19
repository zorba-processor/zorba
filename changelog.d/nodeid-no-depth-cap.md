### Fixed

- Trees may be as deep as memory and the XML parser allow. Node ids had at
  most 255 bytes, about a thousand levels for a chain of single children
  (fewer when elements have preceding siblings or attributes): a deeper
  document failed to load (`FODC0006`, "nodeid requires more than 255 bytes")
  and constructing, copying, validating or updating such a tree raised
  `ZSTR0030`. They now work, at the cost of node ids that grow with depth
  (memory grows with the number of nodes times their depth).
- What walks over a tree recursively no longer crashes on a tree too deep
  for the machine stack (serializing crashed between 10 000 and 20 000
  levels, freeing a loaded tree between 80 000 and 90 000): serializing,
  validating and revalidating, copying (constructors, `copy modify`, inserts
  and replacements, collections), `fn:deep-equal` and the deep-equal hash of
  cached functions, atomizing, `jx:xml-to-json` and full-text tokenization
  raise `zerr:ZXQP0072` with the depth of the node, the document it came from
  and the location of the expression. Freeing a tree, deleting nodes,
  inheriting namespaces after an update, releasing namespace contexts and
  computing base URIs no longer recurse. The serializer and `fn:deep-equal`
  take less stack per level (208 and about 270 bytes instead of 720 and 448).
