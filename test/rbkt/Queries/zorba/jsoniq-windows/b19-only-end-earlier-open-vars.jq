jsoniq version "1.0";
(: As b17: the variables describe the generated window, not the discarded one. :)
let $a := [10, 1, 5]
return [
  for sliding window $w in $a[]
    start $s at $i previous $p next $n when true
    only end $e at $j previous $ep next $en when $e gt $s
  return { "s": $s, "i": $i, "p": $p, "n": $n, "e": $e, "j": $j, "ep": $ep, "en": $en }
]
