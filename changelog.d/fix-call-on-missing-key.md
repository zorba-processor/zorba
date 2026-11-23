### Fixed

- A JSONiq dynamic call on a value that can be absent, such as `$e.h("st")`
  for an object without `h`, gives the empty sequence at optimization levels
  O1 and O2 as at O0, instead of raising XPTY0004 (`empty-sequence() cannot be
  promoted to type function(*)`).
