jsoniq version "1.0";
(: XQuery 3.0 3.10.4.2: overlapping windows of up to three items. :)
let $a := [2, 4, 6, 8, 10, 12, 14]
return [
  for sliding window $w in $a[]
    start at $s when true
    end at $e when $e - $s eq 2
  return [ $w ]
]
