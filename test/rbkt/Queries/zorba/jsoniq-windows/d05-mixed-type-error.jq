jsoniq version "1.0";
(: XQuery 3.0 3.5.2: value comparison of xs:string and xs:integer is a type error. :)
let $a := [ {"v":1}, {"v":"2"} ]
return [ for tumbling window $w in $a[] start $s when $s.v gt 2 return 1 ]
