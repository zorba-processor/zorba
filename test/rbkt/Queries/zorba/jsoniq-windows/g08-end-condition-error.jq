jsoniq version "1.0";
let $a := [ {"v":1,"d":1}, {"v":1,"d":0} ]
return [ for tumbling window $w in $a[] start when true only end $e when $e.v div $e.d gt 100 return 1 ]
