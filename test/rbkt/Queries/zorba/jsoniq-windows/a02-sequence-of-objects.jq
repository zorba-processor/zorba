jsoniq version "1.0";
(: Input: a plain sequence of objects. :)
let $seq := ({"k":1}, {"k":2}, {"k":3})
return [
  for sliding window $w in $seq
    start at $s when true
    only end at $e when $e - $s eq 1
  return [ $w.k ]
]
