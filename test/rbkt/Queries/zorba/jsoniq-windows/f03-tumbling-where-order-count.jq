jsoniq version "1.0";
(: Window followed by let, where, order by and count; $w is read after the sort. :)
let $a := [ {"k":1}, {"k":2}, {"k":3}, {"k":4}, {"k":5}, {"k":6} ]
return [
  for tumbling window $w in $a[]
    start at $s when true
    end at $e when $e - $s eq 1
  let $sum := sum($w.k)
  where $sum gt 3
  order by $sum descending
  count $c
  return { "c": $c, "sum": $sum, "first": $w[1].k, "w": [ $w.k ] }
]
