### Fixed

- At O1 and O2, a dynamic function call or a call of `fn:fold-left`, `fn:fold-right`
  or `fn:for-each-pair` could be moved out of a loop, merged with another call,
  moved to the use of its `let` variable or removed, although the function was
  sequential or nondeterministic or created nodes. In a query where something with
  a side effect is reachable, such a call now stays where it is written unless the
  optimizer can prove the function deterministic and free of side effects, and
  nothing moves across it. In a query with no side effect, it is only kept from
  being evaluated once for several iterations if a nondeterministic function or a
  constructor is reachable.
- In a query where such a call can have a side effect, the domain of a `for` clause
  with the call gives its whole binding sequence first, at all levels, and later
  clauses run per tuple.
- A sliding window clause in a query with a dynamic call and nothing with a side
  effect reachable reads its input lazily and releases it.
- A dynamic function call with an empty sequence as the function raises XPTY0004 in
  XQuery at all levels. In JSONiq, a lookup over an empty sequence stays empty.
- At O1 and O2, a `let` that reads a variable could be moved after a later `let`
  with a direct call of a sequential function, and an argument with such a call
  could run after the part of the function body before the use of the parameter.
