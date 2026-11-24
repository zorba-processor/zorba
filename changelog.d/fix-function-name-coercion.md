### Fixed

- `fn:function-name` of a function item coerced to a function type returns
  the name of the coerced function at every optimization level, also when
  that function is known only at run time, and the empty sequence when the
  coerced function is an inline function or a partial application. Before,
  it returned the generated name of the coercion in these cases: at O0 for
  every coercion of a function passed as an argument, and at every level for
  a coercion of an inline function, of a partial application, or of another
  coercion.
