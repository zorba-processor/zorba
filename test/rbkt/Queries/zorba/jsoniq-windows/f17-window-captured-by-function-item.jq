jsoniq version "1.0";
(: XQuery 3.0 3.1.7: an inline function captures the value of $w when it is created,
   so it can be called after the clause has moved past that window. :)
let $fs := (
  for tumbling window $w in (1, 2, 3, 4)
    start at $s when true
    end at $e when $e - $s eq 1
  return function() { sum($w) })
return [ for $f in $fs return $f() ]
