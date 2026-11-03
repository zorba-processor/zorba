jsoniq version "1.0";
(: Windows follow the order of the binding sequence: unsorted as it arrives,
   sorted after an order by. :)
let $a := [ {"t":3}, {"t":1}, {"t":2} ]
return [
  [ for sliding window $w in $a[] start at $s when true only end at $e when $e - $s eq 1 return [ $w.t ] ],
  [ for sliding window $w in (for $x in $a[] order by $x.t return $x)
      start at $s when true only end at $e when $e - $s eq 1 return [ $w.t ] ]
]
