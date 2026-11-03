jsoniq version "1.0";
let $a := [7]
return [
  [ for tumbling window $w in $a[] start when true return [ $w ] ],
  [ for tumbling window $w in $a[] start when true end when true return [ $w ] ],
  [ for tumbling window $w in $a[] start when true end when false return [ $w ] ],
  [ for tumbling window $w in $a[] start when true only end when true return [ $w ] ],
  [ for tumbling window $w in $a[] start when true only end when false return [ $w ] ],
  [ for sliding window $w in $a[] start when true end when true return [ $w ] ],
  [ for sliding window $w in $a[] start when true end when false return [ $w ] ],
  [ for sliding window $w in $a[] start when true only end when true return [ $w ] ],
  [ for sliding window $w in $a[] start when true only end when false return [ $w ] ],
  [ for tumbling window $w in $a[] start when false return [ $w ] ]
]
