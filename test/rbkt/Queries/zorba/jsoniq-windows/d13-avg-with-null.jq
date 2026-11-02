jsoniq version "1.0";
(: F&O 3.0 14.4.1: fn:avg raises FORG0006 on items that cannot be added. :)
let $a := [ {"v":1}, {"v":null} ]
return [ for tumbling window $w in $a[] start when true end when false return avg($w.v) ]
