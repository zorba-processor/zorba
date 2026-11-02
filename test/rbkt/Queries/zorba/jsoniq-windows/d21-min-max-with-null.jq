jsoniq version "1.0";
(: JSONiq extension 6.16: null is comparable with atomic items and the smallest,
   so F&O 3.0 14.4 fn:min and fn:max can order it. :)
let $a := [ {"v":3}, {"v":null}, {"v":1} ]
return [ for tumbling window $w in $a[] start when true end when false return [ min($w.v), max($w.v) ] ]
