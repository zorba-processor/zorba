jsoniq version "1.0";
(: Windows close at different items and come out in the order of their start items.
   The last window has no end item and ends at the last item. :)
let $a := [1, 5, 2, 3, 9]
return [
  for sliding window $w in $a[]
    start $s when true
    end $e when $e ge $s + 2
  return [ $w ]
]
