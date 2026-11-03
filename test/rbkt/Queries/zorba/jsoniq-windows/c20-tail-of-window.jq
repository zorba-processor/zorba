jsoniq version "1.0";
(: fn:tail and an open-ended range on $w, tumbling and sliding, with items after the window. :)
let $a := [1, 2, 3, 4]
return [
  [ for tumbling window $w in $a[] start at $i when true end at $j when $j - $i eq 1 return [ tail($w) ] ],
  [ for sliding window $w in $a[] start at $i when true only end at $j when $j - $i eq 1 return [ tail($w) ] ],
  [ for tumbling window $w in $a[] start at $i when true end at $j when $j - $i eq 1 return [ $w[position() gt 1] ] ],
  [ for tumbling window $w in $a[] start at $i when true end at $j when $j - $i eq 1 return [ subsequence($w, 2) ] ]
]
