jsoniq version "1.0";
(: Differences between consecutive values with `previous` in the return clause. :)
let $a := [ {"v":10}, {"v":15}, {"v":12} ]
return [
  for sliding window $w in $a[]
    start $s previous $p when exists($p)
    only end when true
  return $s.v - $p.v
]
