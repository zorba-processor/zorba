jsoniq version "1.0";
(: XQuery 3.0 3.10.4.2: without only, a window open at the end of the input is returned. :)
let $a := [1, 2, 3, 4, 5, 6]
return [
  for sliding window $w in $a[]
    start $s when $s mod 2 eq 1
    end $e next $n when $e - $s eq 3
  return [ [ $w ], $s, $e, [ $n ] ]
]
