### Breaking changes

- A function that calls a function declared `%an:nondeterministic` must now
  itself be declared `%an:nondeterministic`, otherwise compilation fails with
  ZXQP0040. Before, the annotation was overwritten by inference when the body
  of the annotated function called no nondeterministic function, and the caller
  compiled.

### Fixed

- A function annotated `%an:nondeterministic` stays nondeterministic when its
  body calls no nondeterministic function. The optimizer no longer moves a call
  of it out of a loop or evaluates it once for several iterations.
