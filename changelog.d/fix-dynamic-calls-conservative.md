### Fixed

- Fix wrong results at O1 and O2 when `reflection:eval` or an invoke function
  runs code with a side effect: an expression beside it in a loop was moved out
  of the loop, and a `let` holding it was removed when its variable was unused.
  An eval now counts as a call that can have a side effect: the query that
  contains it and the query it compiles move nothing out of loops, and a `for`
  domain with an eval gives its whole binding sequence first, at every level.
