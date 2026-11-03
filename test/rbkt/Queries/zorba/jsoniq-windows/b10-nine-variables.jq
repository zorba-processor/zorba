jsoniq version "1.0";
(: XQuery 3.0 3.10.4: all nine variables. Absent previous/next are the empty
   sequence, which an object constructor turns into null (JSONiq 5.1.5). :)
let $a := [2, 4, 6]
return [
  for tumbling window $w in $a[]
    start $s at $sp previous $sprev next $snext when true
    end $e at $ep previous $eprev next $enext when true
  return { "w": [ $w ], "s": $s, "sp": $sp, "sprev": $sprev, "snext": $snext,
           "e": $e, "ep": $ep, "eprev": $eprev, "enext": $enext }
]
