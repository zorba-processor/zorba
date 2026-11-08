### Fixed

- Fix several invalid pointer casts found by the UBSan `vptr` check, now
  enabled in the sanitizer builds: in the `sctx` functions that list in-scope
  schema components, in the full-text `window` and `without content` parse
  nodes, in `StructuralAnyUriItem` node-relationship tests, in
  `CollectionPul::truncateIndexes`, and in the JSound `$minLength` and
  `$maxLength` checks on array types.
- Fix a general index range probe (`probe-index-range-general`) with an
  `xs:untypedAtomic` bound on an index that has a string or `xs:anyURI` key map
  and no numeric key map: it skipped the `xs:dateTime`, `xs:date` and later cast
  attempts. Point probes were not affected.
