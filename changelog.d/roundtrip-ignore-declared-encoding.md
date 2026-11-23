### Fixed

- `jn:decode-from-roundtrip` ignores the encoding that the XML declaration of
  a serialized node names, as `fn:parse-xml` does: the string is already
  decoded. A node declaring `encoding="ISO-8859-1"` turned `é` into `Ã©`.
