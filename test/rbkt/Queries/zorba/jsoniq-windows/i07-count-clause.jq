jsoniq version "1.0";
(: count and where after the window clause. :)
let $a := [1, 2, 3, 4, 5, 6]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $e - $s eq 1
  count $c
  where $c mod 2 eq 1
  return [ $c, [ $w ] ]
]
