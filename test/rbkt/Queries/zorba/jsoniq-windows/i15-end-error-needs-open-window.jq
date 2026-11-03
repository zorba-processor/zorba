jsoniq version "1.0";
(: 1 div 0 at item 3 is not evaluated: no window is open there. :)
let $a := [1, 2, 3, 4, 5, 6]
return [
  for sliding window $w in $a[]
    start $s when $s eq 5
    only end $e when 1 div ($e - 3) gt 0
  return [ $w ]
]
