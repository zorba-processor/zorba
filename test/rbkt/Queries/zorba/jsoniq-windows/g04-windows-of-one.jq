jsoniq version "1.0";
(: XQuery 3.0 3.10.4: if the end item is the start item, the window has one item. :)
let $a := [1, 2, 3]
return [
  [ for tumbling window $w in $a[] start when true end when true return [ $w ] ],
  [ for sliding window $w in $a[] start when true end when true return [ $w ] ],
  [ for sliding window $w in $a[] start when true only end when true return [ $w ] ],
  [ for tumbling window $w in $a[] start when true return [ $w ] ]
]
