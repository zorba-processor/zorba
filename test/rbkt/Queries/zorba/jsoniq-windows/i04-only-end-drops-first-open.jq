jsoniq version "1.0";
(: With only end, the window still open at the end is dropped and the closed ones behind it are returned. :)
let $a := [1, 2, 3, 4]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $s ne 1 and $e eq $s + 1
  return [ $w ]
]
