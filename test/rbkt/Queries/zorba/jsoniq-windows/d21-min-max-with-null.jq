jsoniq version "1.0";
(: F&O 3.0 14.4.3, 14.4.4: fn:max and fn:min accept only types with a total order and
   raise FORG0006 otherwise; the JSONiq specification does not add null to them.
   Filter it first, as in d11. :)
let $a := [ {"v":3}, {"v":null}, {"v":1} ]
return [ for tumbling window $w in $a[] start when true end when false return [ min($w.v), max($w.v) ] ]
