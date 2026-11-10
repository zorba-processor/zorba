### Fixed

- A function annotated `%an:nondeterministic` stays nondeterministic when its
  body calls no nondeterministic function. The optimizer no longer moves a call
  of it out of a loop or evaluates it once for several iterations.
