jsoniq version "1.0";
(: A let over the window variable and a later for read the window several times in one window. :)
let $a := [1, 2, 3]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $e - $s eq 1
  let $x := $w
  for $j in 1 to 2
  let $y := $x[$j]
  return $y * $j
]
