### Fixed

- Fix a crash after `import schema` named a schema document whose target
  namespace was already loaded, when that document imports, directly or
  transitively, a namespace not yet loaded whose components refer to it.
- Fix `import schema` of a namespace whose location hint names a schema
  document with another target namespace: it now raises `XQST0059` instead of
  loading that document's namespace.

### Changed

- The location hint of an `import schema` whose namespace is already loaded
  (by an earlier import or a schema it imported) is disregarded; the namespace
  keeps the components loaded first, and schema documents named only by that
  hint are not read.
