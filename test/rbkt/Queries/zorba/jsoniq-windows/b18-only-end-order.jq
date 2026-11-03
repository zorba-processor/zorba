jsoniq version "1.0";
(: XQuery 3.0 3.10.4.3: tuples follow the order of the start items, although the
   window that starts at 1 closes before the one that starts at 10. :)
let $a := [10, 1, 5, 11]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $e gt $s
  return [ $w ]
]
