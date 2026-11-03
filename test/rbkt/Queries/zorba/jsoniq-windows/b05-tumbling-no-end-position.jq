jsoniq version "1.0";
(: XQuery 3.0 3.10.4.1: no end clause, a window lasts until the next start item. :)
let $a := [2, 4, 6, 8, 10, 12, 14]
return [
  for tumbling window $w in $a[]
    start at $s when $s mod 3 eq 1
  return [ $w ]
]
