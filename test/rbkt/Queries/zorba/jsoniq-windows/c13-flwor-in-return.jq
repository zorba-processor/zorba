jsoniq version "1.0";
let $a := [ {"ts":"a","v":1}, {"ts":"b","v":3}, {"ts":"c","v":2} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return [ for $x in $w where $x.v gt 1 order by $x.v descending return $x.ts ]
]
