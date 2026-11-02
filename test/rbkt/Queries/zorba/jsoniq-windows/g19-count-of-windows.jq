jsoniq version "1.0";
(: XQuery 3.0 3.10.4.3: the FLWOR gives one item per window, also when its return
   clause uses no variable. No window never closes: none is generated with `only end`. :)
let $a := [1, 2, 3]
return [
  count(for sliding window $w in $a[] start when true only end $e when $e lt 0 return 1),
  count(for sliding window $w in $a[] start $s when true only end $e when $e lt 0 return $s),
  count(for tumbling window $w in $a[] start when true only end $e when $e lt 0 return 1),
  count(for tumbling window $w in $a[] start when false return 1),
  count(for sliding window $w in $a[] start when true end when true return 1),
  count(for tumbling window $w in $a[] start when true return 1),
  exists(for sliding window $w in $a[] start when true only end $e when $e lt 0 return 1),
  empty(for sliding window $w in $a[] start when true only end $e when $e lt 0 return 1)
]
