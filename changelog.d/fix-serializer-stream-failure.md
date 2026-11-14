### Fixed

- Serializing to a C++ output stream whose writes fail without throwing (for
  example a full disk, with `badbit` not in the stream's exceptions mask) now
  raises `zerr:ZOSE0004` instead of returning normally with the text cut off.
  The serializer flushes the stream at the end of each serialization; streams
  that throw on `badbit` keep throwing their own exception.
