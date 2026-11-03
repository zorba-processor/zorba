jsoniq version "1.0";
(: Input: $a[] over an array. XQuery 3.0 3.10.4.1: `only end` drops the last, unfinished window. :)
let $a := [ {"k":1}, {"k":2}, {"k":3}, {"k":4}, {"k":5} ]
return [
  for tumbling window $w in $a[]
    start at $s when true
    only end at $e when $e - $s eq 1
  return [ $w.k ]
]
