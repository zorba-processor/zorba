### Added

- Add a hint to errors that come from a form valid elsewhere: `ZorbaException::hint()` holds what to write instead,
  `zorba` prints it after the message and `zorba-mcp` returns it in a separate `hint` field. Error codes and messages
  do not change. Hints cover `$x[]`, `$$`, `$x[[]]`, `keys()`/`size()`/`flatten()`/`members()`/`pi()` without a prefix in
  XQuery, `$a-$b` and `$x-1`, a function called with the wrong number of arguments (with its declared forms), the
  prefixes `math:` and `map:`, a path over a JSON object or array (XPTY0019), a string selector on an array (FORG0001)
  and a node in JSON output (JNSE0014).

### Changed

- `zorba-mcp` predeclares the namespace prefix `math:`; a query can still import it or bind it to another namespace.
