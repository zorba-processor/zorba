jsoniq version "1.0";
(: XQuery 3.0 3.10.4: with `only end` a start item with no end item generates no
   window. 10 has no later item above it, 5 neither; 1 ends at 5. :)
let $a := [10, 1, 5]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $e gt $s
  return [ $w ]
]
