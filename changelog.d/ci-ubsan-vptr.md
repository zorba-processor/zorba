### Fixed

- Fix several invalid pointer casts found by the UBSan `vptr` check, now
  enabled in the sanitizer builds: in the `sctx` functions that list in-scope
  schema components, in the full-text `window` and `without content` parse
  nodes, in `StructuralAnyUriItem` node-relationship tests, in
  `CollectionPul::truncateIndexes`, and in the JSound `$minLength` and
  `$maxLength` checks on array types.
- Fix a probe of a general index with an `xs:untypedAtomic` value that could
  skip the `xs:dateTime`, `xs:date` and later cast attempts when the index had
  no numeric key map.
