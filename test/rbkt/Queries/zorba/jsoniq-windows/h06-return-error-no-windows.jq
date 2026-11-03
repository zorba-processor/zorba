jsoniq version "1.0";
(: The return clause does not run without tuples, whatever can be hoisted from it. :)
let $z := {"d":0}
let $a := [1, 2]
return [
  for tumbling window $w in $a[]
    start $s when $s lt 0
  return 1 div $z.d
]
