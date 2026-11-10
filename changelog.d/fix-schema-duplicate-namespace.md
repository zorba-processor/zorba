### Fixed

- Fix a crash after `import schema` of a namespace that an earlier imported
  schema document already loaded, when the schema document named by that
  import and a schema it imports import each other: the location hint of such
  an import is now disregarded, and the namespace keeps the components loaded
  first.
