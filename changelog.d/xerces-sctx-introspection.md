### Fixed

- In builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, the schema introspection
  functions of the sctx module (`sctx:in-scope-element-declarations`,
  `-attribute-declarations`, `-element-groups`, `-attribute-groups` and
  `-schema-types`) no longer crash when they run on several threads at once or
  while a schema is imported into the same static context. Each call reads the
  schema's components once, under the Xerces lock, and returns that snapshot.
