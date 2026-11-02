jsoniq version "1.0";
(: The same for sliding windows, where several windows end at the last item. :)
let $a := [1, 2, 3]
return [
  for sliding window $w in $a[]
    start $s at $sp previous $sprev next $snext when true
    end $e at $ep previous $eprev next $enext when $ep - $sp eq 1
  return { "w": [ $w ], "s": $s, "sp": $sp, "sprev": $sprev, "snext": $snext,
           "e": $e, "ep": $ep, "eprev": $eprev, "enext": $enext }
]
