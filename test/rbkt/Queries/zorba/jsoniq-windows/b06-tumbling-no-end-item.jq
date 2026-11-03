jsoniq version "1.0";
(: XQuery 3.0 3.10.4.1: windows that start at a number divisible by 3. :)
let $a := [2, 4, 6, 8, 10, 12, 14]
return [
  for tumbling window $w in $a[]
    start $first when $first mod 3 eq 0
  return [ $w ]
]
