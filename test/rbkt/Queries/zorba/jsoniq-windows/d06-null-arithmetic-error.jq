jsoniq version "1.0";
(: JSONiq extension 6.15: arithmetic on null is a type error. :)
let $a := [ {"v":null} ]
return [ for tumbling window $w in $a[] start $s when $s.v + 1 gt 0 return 1 ]
