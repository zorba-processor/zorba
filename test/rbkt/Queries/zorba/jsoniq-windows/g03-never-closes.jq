jsoniq version "1.0";
(: XQuery 3.0 3.10.4: no end item: with `only end` no window, otherwise it ends at the last item. :)
let $a := [1, 2, 3]
return [
  [ for tumbling window $w in $a[] start when true only end when false return [ $w ] ],
  [ for tumbling window $w in $a[] start when true end when false return [ $w ] ],
  [ for sliding window $w in $a[] start when true only end when false return [ $w ] ],
  [ for sliding window $w in $a[] start when true end when false return [ $w ] ]
]
