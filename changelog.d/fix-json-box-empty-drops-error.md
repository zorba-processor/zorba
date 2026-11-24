### Fixed

- At optimization levels O1 and O2, an object value or replacement value of
  the empty sequence type is replaced by `null` only when it cannot raise an
  error. Before, `{ "a" : if (6 idiv $zero eq 1) then () else () }` with
  `$zero` 0 returned `{ "a" : null }` at O1 and O2 and raised FOAR0001 at O0;
  it now raises FOAR0001 at every level.
