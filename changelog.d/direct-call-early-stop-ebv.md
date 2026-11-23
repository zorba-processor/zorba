### Changed

- `fn:empty`, `fn:boolean`, `fn:not`, the condition of an `if` (the effective
  boolean value) and `fn:zero-or-one` over a loop with a direct
  sequential call stop after the items they need, as `fn:head` and `fn:exists`
  do: `(empty(for $i in 1 to 3 return local:bump()), $g)` gives `false 1`
  instead of `false 3`. The effective boolean value and `fn:zero-or-one` read
  up to the second item, the one that makes them raise FORG0006 or FORG0003,
  so the iterations after it do not run.
