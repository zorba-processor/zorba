jsoniq version "1.0";
(: XQuery 3.0 3.10.4: an end clause without `only` that is never true sets the
   end item to the last item, so the end variables describe that item. :)
let $a := [10, 20, 30]
return [
  for tumbling window $w in $a[]
    start $s at $sp previous $sprev next $snext when true
    end $e at $ep previous $eprev next $enext when $ep - $sp eq 1
  return { "w": [ $w ], "s": $s, "sp": $sp, "sprev": $sprev, "snext": $snext,
           "e": $e, "ep": $ep, "eprev": $eprev, "enext": $enext }
]
