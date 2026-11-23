### Changed

- The type names `xs:anyType`, `xs:untyped`, `xs:anySimpleType`,
  `xs:NMTOKENS`, `xs:IDREFS`, `xs:ENTITIES` and the JSONiq `null` type name
  held by the type system are no longer reference-counted, like the store's
  type names, so in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF` queries on
  several threads no longer all write the same counts.
