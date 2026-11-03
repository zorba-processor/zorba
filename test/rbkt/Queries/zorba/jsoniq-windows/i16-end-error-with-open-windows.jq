jsoniq version "1.0";
(: 1 div 0 at item 3 with open windows: the condition is evaluated. :)
let $a := [1, 2, 3, 4, 5, 6]
return [
  for sliding window $w in $a[]
    start when true
    only end $e when 1 div ($e - 3) gt 5
  return [ $w ]
]
