jsoniq version "1.0";
(: A navigation on an outer variable in a condition, which hoisting can move out. :)
let $cfg := {"n":2}
return [
  for sliding window $w in (1, 2, 3, 4)
    start at $s when true
    only end at $e when $e - $s eq $cfg.n
  return [ $w ]
]
