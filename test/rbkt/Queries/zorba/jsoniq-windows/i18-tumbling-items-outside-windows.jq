jsoniq version "1.0";
(: Items before and between tumbling windows are released while previous stays readable. :)
let $a := [1, 2, 3, 4, 5, 6, 7, 8]
return [
  [ for tumbling window $w in $a[]
      start $s previous $p when $s mod 4 eq 3
      end $e next $n when $e eq $s
    return [ [ $w ], [ $p ], [ $n ] ] ],
  [ for tumbling window $w in $a[]
      start $s previous $p when $s mod 4 eq 3
    return [ [ $w ], [ $p ] ] ]
]
