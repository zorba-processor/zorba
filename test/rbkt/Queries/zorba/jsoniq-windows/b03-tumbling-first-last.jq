jsoniq version "1.0";
(: XQuery 3.0 3.10.4.1: first and last items of each window. :)
let $a := [2, 4, 6, 8, 10, 12, 14]
return [
  for tumbling window $w in $a[]
    start $first at $s when true
    only end $last at $e when $e - $s eq 2
  return [ $first, $last ]
]
