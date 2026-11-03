jsoniq version "1.0";
(: `only end`: end variables of closed windows. :)
let $a := [1, 2, 3, 4]
return [
  for tumbling window $w in $a[]
    start at $i when true
    only end $e at $j previous $ep next $en when $j - $i eq 1
  return { "w": [ $w ], "e": $e, "j": $j, "ep": $ep, "en": $en }
]
