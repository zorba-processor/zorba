### Fixed

- Fix hash joins raising an error that the query without the join index does
  not raise: the outer key was evaluated over an empty inner domain, the inner
  keys under an empty outer domain, after `fn:exists()` had its first item, and
  for nodes that an earlier conjunct of the `where` clause rejects. An index
  build or outer key that raises now makes the join evaluate its comparison as
  without the index, and a join comparison is hash-joined only when it is the
  first conjunct of its `where` clause and only `let` clauses of hoisted
  expressions (which keep an error until the value is used) lie between the
  inner `for` clause and the `where` clause. An assertion or internal error of
  the engine raised by the index or its probe is not caught.
