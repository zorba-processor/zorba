jsoniq version "1.0";
let $a := [1, 2, 3]
return [
  for tumbling window $w in $a[]
    start next $n when empty($n)
  return [ $w ]
]
