jsoniq version "1.0";
(: Null is smaller than 50 (JSONiq 5.2.3), a missing value compares as empty. :)
let $n := [ {"i":1,"v":40}, {"i":2,"v":null}, {"i":3,"v":60} ]
let $m := [ {"i":1,"v":40}, {"i":2}, {"i":3,"v":60} ]
return [
  [ for tumbling window $w in $n[] start $s previous $p when $p.v lt 50 and $s.v ge 50 return [ $w.i ] ],
  [ for tumbling window $w in $m[] start $s previous $p when $p.v lt 50 and $s.v ge 50 return [ $w.i ] ]
]
