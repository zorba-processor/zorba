jsoniq version "1.0";
(: Sliding windows after items were released: every neighbor of start and end is still bound. :)
let $a := [1, 2, 3, 4, 5]
return [
  for sliding window $w in $a[]
    start $s previous $p next $n when true
    only end $e previous $q next $r when $e - $s eq 1
  return [ [ $w ], $s, [ $p ], [ $n ], $e, [ $q ], [ $r ] ]
]
