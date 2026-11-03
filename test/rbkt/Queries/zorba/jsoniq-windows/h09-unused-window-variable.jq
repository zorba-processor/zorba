jsoniq version "1.0";
(: The windows exist whether or not $w is used. :)
let $a := [1, 2, 3]
return [
  for tumbling window $w in $a[]
    start at $i when $i gt 1
  return "x"
]
