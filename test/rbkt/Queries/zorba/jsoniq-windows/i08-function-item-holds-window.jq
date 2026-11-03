jsoniq version "1.0";
(: A function item called after the clause left its window behind and released the items. :)
let $fs := (
  for sliding window $w in (1, 2, 3, 4, 5)
    start $s when true
    only end $e when $e - $s eq 1
  return function($k) { $w[$k] })
return [ for $f in $fs return [ $f(1), $f(2), $f(3) ] ]
