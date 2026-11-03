jsoniq version "1.0";
(: A position computed when the function is called, on a window the clause has left behind. :)
let $fs := (
  for tumbling window $w in (1, 2, 3, 4, 5)
    start at $s when true
    end at $e when $e - $s eq 1
  return function($k) { $w[$k] })
return [ for $f in $fs return [ $f(1), $f(2), $f(3) ] ]
