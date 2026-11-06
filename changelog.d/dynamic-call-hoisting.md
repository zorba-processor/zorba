### Fixed

- At O1 and O2, a dynamic function call or a call of `fn:fold-left`, `fn:fold-right`
  or `fn:for-each-pair` could be moved out of a loop, merged with another call,
  moved to the use of its `let` variable or removed, although the function was
  sequential or nondeterministic or created nodes. In a query where something with
  a side effect is reachable, such a call now stays where it is written unless the
  optimizer can prove the function deterministic and free of side effects, and an
  expression that reads a state the call can change stays with it; if a node or
  JSON item can be updated, nothing is hoisted in the query body or function body
  that has such a call. In a query with no side effect, it is only kept from
  being evaluated once for several iterations if a nondeterministic function or a
  constructor is reachable.
- In a query where such a call can have a side effect, the domain of a `for` clause
  with the call gives its whole binding sequence first, at all levels, and later
  clauses run per tuple. This is decided before the optimizer runs, so a `for`
  clause that only returns its variable, or a call that inlining proves pure, gives
  the same result at O0, O1 and O2.
- An index join is not used when the domain of the inner variable or the join
  predicate has a dynamic call whose result can differ per call.
- A sliding window clause in a query with a dynamic call and nothing with a side
  effect reachable reads its input lazily and releases it.
- A dynamic function call with an empty sequence as the function raises XPTY0004 in
  XQuery at all levels, with or without arguments; this includes a lookup such as
  `$o("a")("b")` or `$o("a")()` when `$o("a")` is empty, which gave an empty result.
  In JSONiq, a lookup over an empty sequence stays empty, also at O1 and O2 when the
  optimizer finds that the sequence has at most one item.
- At O1 and O2, a `let` that reads a variable could be moved after a later `let`
  with a direct call of a sequential function, and an argument with such a call
  could run after the part of the function body before the use of the parameter.
