jsoniq version "1.0";
(: Records out of order detected with `previous`. :)
let $a := [ {"t":1}, {"t":3}, {"t":2}, {"t":4}, {"t":0} ]
return [
  for tumbling window $w in $a[]
    start $s previous $p when $s.t lt $p.t
  return $s.t
]
