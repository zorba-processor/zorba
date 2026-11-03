jsoniq version "1.0";
(: order by reads every window before it returns the first one. :)
let $a := [1, 2, 3, 4, 5, 6]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $e - $s eq 1
  order by $s descending
  return [ $w ]
]
