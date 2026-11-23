### Changed

- Parse-time projection of `parse-json` at `-O1` and `-O2` reaches more
  queries: a parse returned by a user function (inlined, see below), a
  `window` clause over a parse (the window and its start and end items),
  values taken by position (`$e[1].time`), values that a FLWOR or a predicate
  gives to an enclosing lookup (`parse-json($s, $o)[$$.level eq "error"].msg`)
  or to an enclosing `for` clause, and a lookup of a literal key directly in a
  parse (`max(parse-json($s, $o).ms)`) build only the keys they read.
- At `-O1` and `-O2`, a user function whose body calls no user function once
  optimized is inlined, also when it called one before (a body calling a
  function that was inlined into it); before, only functions that called no
  user function at all were.

### Fixed

- A call of a function annotated `%an:nondeterministic` whose body calls no
  nondeterministic function is no longer inlined at `-O1` and `-O2`, where the
  inlined body lost the annotation and could be hoisted out of a loop.
