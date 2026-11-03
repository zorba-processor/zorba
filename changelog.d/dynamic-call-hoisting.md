### Fixed

- At O1 and O2, a dynamic function call or a call of `fn:fold-left`, `fn:fold-right`
  or `fn:for-each-pair` could be moved out of a loop, merged with another call,
  moved to the use of its `let` variable or removed, although the function was
  sequential or nondeterministic or created nodes. The optimizer now does this only
  if it can see the function and prove that it is deterministic and free of side
  effects. Otherwise nothing moves across the call, so results are the same at O0,
  O1 and O2.
- A FLWOR expression where such a call can be observed by another clause runs each
  clause for all the tuples before the next, as with a direct call of a sequential
  function.
- At O1 and O2, a `let` that reads a variable could be moved after a later `let`
  with a direct call of a sequential function, and an argument with such a call
  could run after the part of the function body before the use of the parameter.
