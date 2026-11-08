### Fixed

- Fix wrong results of `zq:evaluate` in a loop at optimization levels O1 and
  O2 when the prepared query has a side effect through a function item: the
  optimizer treated the call as pure and could run it once for all iterations,
  or move an expression beside it out of the loop. A call of `zq:evaluate` is
  now treated like a call of `fn:invoke` or `r:eval`, whose code the compiler
  cannot see.
