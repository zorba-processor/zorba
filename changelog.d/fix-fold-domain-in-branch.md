### Fixed

- At O1 and O2, a `for` variable whose domain has at most one item and can
  raise an error is folded only into a position that is evaluated for every
  tuple: the expression of a later `for`, `window` or `where` clause or the
  return clause, reached through the condition of an `if`, the first operand
  of an arithmetic operator, a general comparison or a value comparison whose
  other operand is never empty, `fn:data`, a cast, the first step
  of a path, a positional predicate at a constant position of at least 1, the
  function of a dynamic call, a value captured by a function item, or the
  clauses of a nested FLWOR that keep every tuple. Before, it was also folded
  into an `if` branch, a `let` binding, the second operand of `or` or `and`, a
  predicate or the return clause of a nested FLWOR that can have no tuples,
  where it could go unevaluated and the error that the query raises at O0 was
  lost.
