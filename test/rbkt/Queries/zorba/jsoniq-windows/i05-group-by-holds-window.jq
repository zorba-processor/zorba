jsoniq version "1.0";
(: group by keeps the window variable after its tuple moved on and items were released. :)
let $a := [1, 2, 3, 4, 5, 6]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $e - $s eq 1
  group by $k := $s mod 2
  order by $k
  return [ $k, [ $w ] ]
]
