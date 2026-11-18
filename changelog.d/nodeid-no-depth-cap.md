### Fixed

- Trees may be as deep as memory and the XML parser allow. Node ids had at
  most 255 bytes, about a thousand levels for a chain of single children
  (fewer when elements have preceding siblings or attributes): a deeper
  document failed to load (`FODC0006`, "nodeid requires more than 255 bytes")
  and constructing, copying, validating or updating such a tree raised
  `ZSTR0030`. They now work, at the cost of node ids that grow with depth
  (memory grows with the number of nodes times their depth).
