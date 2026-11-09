### Fixed

- Fix building a text node under a parent when an allocation fails while the
  node is attached: debug and sanitizer builds stopped on an assertion, and
  release builds did not free the text of the node.
