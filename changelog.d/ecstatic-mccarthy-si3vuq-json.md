### Changed

- Reword the JSON and JSONiq error messages: JNDY0003, JNTY0004, JNTY0011, JNTY0018, JNTY0020, JNTY0024, JNUP0017,
  JNSE0013 and the "illegal JSON literal" parse error (JNDY0021, ZJPE0004), which now lists the valid literals.
  JNSE0012 says to wrap the items in an array or set the serialization parameter `jsoniq-multiple-items` to `yes`.
  Error codes do not change.

### Fixed

- JNTY0020 from `jn:parse-json()` left the option name out of the message.
