jsoniq version "1.0";
(: JSONiq 5.5.2: [] iterates over the left-hand sequence. :)
let $a := [ [1,2], [3,4], [5,6] ]
return [
  for tumbling window $w in $a[]
    start at $i when true
    end at $j when $j - $i eq 1
  return [ $w[] ]
]
