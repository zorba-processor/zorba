jsoniq version "1.0";
(: Positional reads of a sliding window, none of which may reach a released item or a size. :)
let $a := [1, 2, 3, 4, 5]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $e - $s eq 2
  for $i in 2
  return [ $w[1], $w[last()], [ $w[position() gt 1] ], [ subsequence($w, 2, 2) ],
           [ subsequence($w, 2) ], count($w), [ $w[4] ], $w[$i], [ $w[$i - 2] ] ]
]
