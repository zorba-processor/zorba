jsoniq version "1.0";
(: A function item called after the clause left its window behind. The dynamic call keeps this input
   eager, so nothing is released here: this fails only if the capture stops being a copy of the window.
   c22 is the same case with the items released, on a tumbling window. :)
let $fs := (
  for sliding window $w in (1, 2, 3, 4, 5)
    start $s when true
    only end $e when $e - $s eq 1
  return function($k) { $w[$k] })
return [ for $f in $fs return [ $f(1), $f(2), $f(3) ] ]
