### Fixed

- `jn:decode-from-roundtrip` of an object whose type value is neither a node
  kind nor the name of a type in scope (such as `"document()"`, `"xs:unknown"`,
  `""` or `"Q{urn:x"`) raises `jerr:JNTY0023` at the call. It crashed the
  process.
