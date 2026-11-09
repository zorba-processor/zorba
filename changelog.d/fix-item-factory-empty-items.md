### Fixed

- Fix crashes in the C++ API `ItemFactory` when an empty item is passed as the
  node name of `createElementNode` or `createAttributeNode`, as the element or
  the typed value of `assignElementTypedValue`, as the typed value of
  `createAttributeNode`, or inside the arguments of `createJSONObject` and
  `createJSONArray`; they now raise `ZAPI0014`. An empty parent of the node
  constructors stays allowed.
