(: A constructor in the end condition makes a node with a new identity at each evaluation.
   A window closes on its own evaluation, so two overlapping windows end at different items. With one evaluation
   per item, all the open windows end together. p = 1/2 per evaluation: about 10^-11 to see no difference by chance. :)
let $p :=
  for sliding window $w in 1 to 64
    start at $s when true
    only end $x at $e when string-to-codepoints(generate-id(<a>{$x}</a>))[last()] mod 2 eq 0
  return $s * 1000 + $e
return some $a in $p, $b in $p satisfies
  ($a idiv 1000 lt $b idiv 1000 and $b idiv 1000 le $a mod 1000 and $a mod 1000 ne $b mod 1000)
