### Fixed

- Fix a crash in the C++ API `ItemFactory::createUserTypedAtomicItem` when the
  base item or the type name is an empty item; it now raises `ZAPI0014`.
