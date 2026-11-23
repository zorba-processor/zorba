### Fixed

- Sealing the functions that a compiled query declares but does not call
  (`XQuery::getStaticContext()`, `XQuery::clone()`, and the `zorba` command
  when it serializes the result) no longer changes the recursion information
  of a function that the query sealed before. A Debug build aborted there when
  such a function called a recursive function that the query calls; a Release
  build added duplicate entries to the sealed function.
