### Fixed

- Concurrent XML parses on libxml2 older than 2.14 no longer race when they
  install the entity loader that enforces the security policy.
- A Debug build prints nothing to standard output when it converts an XQueryX
  query.
- A Debug build with `-Werror` compiles: the regex translator marks its
  intended fallthrough, and the memory-size unit test no longer reads an
  uninitialized variable.
