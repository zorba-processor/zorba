jsoniq version "1.0";
(: Start variables of windows closed by the next start item and by the end of the input. :)
let $a := [5, 6, 7]
return [
  for tumbling window $w in $a[]
    start $s at $p previous $pr next $n when $p mod 2 eq 1
  return { "w": [ $w ], "s": $s, "p": $p, "pr": $pr, "n": $n }
]
