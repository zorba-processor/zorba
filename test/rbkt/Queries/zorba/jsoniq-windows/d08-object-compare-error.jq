jsoniq version "1.0";
(: JSONiq extension 6.3: atomization of an object raises JNTY0004. :)
let $a := [ {"v":{"a":1}} ]
return [ for tumbling window $w in $a[] start $s when $s.v eq 1 return 1 ]
