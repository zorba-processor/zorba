jsoniq version "1.0";
(: The first window closes after later ones, which wait for it and keep the items it needs. :)
let $a := [1, 2, 3, 4, 5, 6]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when ($s eq 1 and $e eq 5) or ($s ne 1 and $e eq $s + 1)
  return [ $w ]
]
