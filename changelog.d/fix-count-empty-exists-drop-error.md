### Fixed

- At optimization levels O1 and O2, `fn:count`, `fn:empty` and `fn:exists`
  are replaced by their value from the static type of their argument only
  when the argument cannot raise an error (such as a constant or a reference to a
  for, positional, count or initialized prolog variable). Before,
  `count(6 idiv $zero)` with `$zero` 0 returned 1 at O1 and O2 and raised
  FOAR0001 at O0; it now raises FOAR0001 at every level, as do
  `empty(6 idiv $zero)` and `exists(if (6 idiv $zero eq 1) then () else ())`.
