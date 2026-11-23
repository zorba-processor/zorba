### Changed

- A general `=` with a `let` variable on one side, evaluated again for the same
  value of the variable (for example `not($e.id = $seen)` in a `where` clause,
  with `$seen` bound before the `for`), hashes the items of the variable once
  and looks up each item of the other side, instead of comparing every pair.
  The answer and the errors stay those of comparing every pair: an item that
  could raise an error against some item of the variable (an `xs:string`
  against a number, an `xs:untypedAtomic` against a number) is compared pair
  by pair, in the same order as before.
