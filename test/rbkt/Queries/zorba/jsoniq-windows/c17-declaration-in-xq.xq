jsoniq version "1.0";
(: The declaration selects JSONiq whatever the extension of the query. :)
let $a := [ {"v":1}, {"v":2}, {"v":3} ]
return [
  for tumbling window $w in $a[]
    start at $i when true
    end at $j when $j - $i eq 1
  return sum($w.v)
]
