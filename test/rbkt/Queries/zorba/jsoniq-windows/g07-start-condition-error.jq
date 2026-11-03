jsoniq version "1.0";
(: F&O 3.0 4.2: integer division by zero raises FOAR0001, from a condition too. :)
let $a := [ {"v":1} ]
return [ for tumbling window $w in $a[] start $s when $s.v div 0 gt 1 return 1 ]
