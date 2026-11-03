jsoniq version "1.0";
(: JSONiq extension 6.1: true if the first item of the sequence is an object or an array. :)
let $a := [ {"a":1}, {"a":2} ]
return [ for tumbling window $w in $a[] start $s when ($s, 1) return $s.a ]
