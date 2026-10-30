let $square := function($x) { $x * $x }
let $even := function($x) { $x mod 2 eq 0 }
let $nums := 1 to 6
return (
  for-each($nums, $square),
  filter($nums, $even),
  fold-left($nums, 0, function($acc, $x) { $acc + $x }),
  for-each(("a", "b"), upper-case#1),
  $square($square(3))
)
