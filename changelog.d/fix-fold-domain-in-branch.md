### Fixed

- At O1 and O2, a `for` variable whose domain has at most one item and can
  raise an error is folded only into a position that is evaluated for every
  tuple: the expression of a later `for`, `window` or `where` clause, the
  return clause, the condition of an `if`, or the first operand of an
  arithmetic operator or a comparison inside one of them. Before, it was also
  folded into an `if` branch, a `let` binding, the second operand of `or` or
  `and`, a predicate or the return clause of a nested FLWOR that can have no tuples, where it could go unevaluated and the
  error that the query raises at O0 was lost.
