### Fixed

- A general range index without a key type, or with `xs:anyAtomicType*` as
  its key type, now raises `zerr:ZDTY0012` when a key is a QName, NOTATION,
  binary or `xs:gYear`-like value, as documented, both when the index is
  built and when it is maintained. Release builds used to accept such keys.
