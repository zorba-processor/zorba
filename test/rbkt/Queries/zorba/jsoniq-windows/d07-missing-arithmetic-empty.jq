jsoniq version "1.0";
(: JSONiq 5.2.1: arithmetic on the empty sequence is empty, hence false in `when`. :)
let $a := [ {"i":1}, {"i":2,"v":1} ]
return [ for tumbling window $w in $a[] start $s when $s.v + 1 gt 0 return [ $w.i ] ]
