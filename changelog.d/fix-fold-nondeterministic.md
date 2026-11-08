### Fixed

- Fix wrong results at optimization levels O1 and O2 when a call of a
  nondeterministic function was dropped because its static type decided the
  result: for example `fn:exists(ref:reference($n))` became `true()` without
  calling `ref:reference`, so a later `ref:has-reference($n)` returned false.
  A rewrite no longer drops an expression that contains such a call (`fn:exists`,
  `fn:empty`, `fn:count`, `fn:boolean`, `instance of`, `castable as`, `and`/`or`
  with a constant operand, unused `for` and `let` variables, a `for` over an
  empty sequence type, and an empty object value), and `count(E) = 0`, `= 1` or
  `= n` no longer reads only part of such an E.
