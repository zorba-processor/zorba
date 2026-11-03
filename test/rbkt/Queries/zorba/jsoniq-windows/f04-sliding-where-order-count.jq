jsoniq version "1.0";
let $a := [ {"k":1}, {"k":2}, {"k":3}, {"k":4} ]
return [
  for sliding window $w in $a[]
    start at $s when true
    only end at $e when $e - $s eq 1
  let $sum := sum($w.k)
  where $sum gt 3
  order by $sum descending
  count $c
  return { "c": $c, "sum": $sum, "w": [ $w.k ] }
]
