### Changed

- A general `=` on a parameter of a function that is not inlined, in calls
  that pass the same value of a `let`, global or local variable, hashes the
  items once and looks up each item of the other side, as it already did for
  the variable itself. The comparison under `fn:data()` (an untyped
  parameter) is hashed when the items are atomic. A parameter used more than
  once shares the materialized value of the variable instead of copying it on
  each call. The answer and the errors stay those of comparing every pair.
