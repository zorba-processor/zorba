### Fixed

- Fix a sequential inline function item that declares local variables reading
  the value of one of them for the last variable it captures: the first local
  variable of its body had the same dynamic-context id as that captured
  variable.
