### Fixed

- Fix hash joins raising an error that the query without the join index does
  not raise: the outer key was evaluated over an empty inner domain, the inner
  keys under an empty outer domain, after `fn:exists()` had its first item, and
  for nodes that an earlier conjunct of the `where` clause rejects. An index
  build or outer key that raises now makes the join evaluate its comparison as
  without the index, and a join comparison is hash-joined only when it is the
  first conjunct of a `where` clause that directly follows the inner `for`
  clause.
