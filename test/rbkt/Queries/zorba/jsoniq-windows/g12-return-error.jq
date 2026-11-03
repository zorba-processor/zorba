jsoniq version "1.0";
(: An error in the return clause of the second window. :)
let $a := [1, 2]
return [ for tumbling window $w in $a[] start $s when true return 1 div ($s - 2) ]
