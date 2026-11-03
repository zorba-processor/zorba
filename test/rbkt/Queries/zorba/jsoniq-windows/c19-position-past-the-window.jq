jsoniq version "1.0";
(: XQuery 3.0 3.10.4: the window variable holds only the items of the window, so
   a position past its end selects nothing, even if the input has more items. :)
let $a := [1, 2, 3]
return [
  [ for tumbling window $w in $a[] start when true end when true return [ $w[2] ] ],
  [ for sliding window $w in $a[] start when true end when true return [ $w[2] ] ],
  [ for tumbling window $w in $a[] start at $i when true end when true let $k := $i - $i + 2 return [ $w[$k] ] ],
  [ for sliding window $w in $a[] start at $i when true end when true let $k := $i - $i + 2 return [ $w[$k] ] ],
  [ for tumbling window $w in $a[] start when true end when true return [ subsequence($w, 2, 1) ] ],
  [ for tumbling window $w in $a[] start when true end when true return [ $w[position() eq 2] ] ]
]
