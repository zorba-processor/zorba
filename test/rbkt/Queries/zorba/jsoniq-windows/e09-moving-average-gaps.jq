jsoniq version "1.0";
(: Moving average with a missing value: over the records that have one in each
   window of 2 records, or over 2 present values when the domain is filtered first. :)
let $a := [ {"v":1.5}, {}, {"v":2.5}, {"v":3.5} ]
return [
  [ for sliding window $w in $a[] start at $s when true only end at $e when $e - $s eq 1 return avg($w.v) ],
  [ for sliding window $w in $a[][exists($$.v)] start at $s when true only end at $e when $e - $s eq 1 return avg($w.v) ]
]
