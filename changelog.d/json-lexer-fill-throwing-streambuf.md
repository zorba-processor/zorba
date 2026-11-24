### Fixed

- Fix the JSON lexer losing the characters that a stream buffer without a get area
  had already given when it threw while the lexer was filling its buffer: they
  are lexed first, and the failure is reported after them.
