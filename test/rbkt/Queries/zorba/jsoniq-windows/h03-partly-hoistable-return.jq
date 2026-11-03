jsoniq version "1.0";
let $refs := [ {"k":1}, {"k":3} ]
let $a := [ {"v":10}, {"v":20}, {"v":30} ]
return [
  for tumbling window $w in $a[]
    start at $i when true
    end at $j when $j - $i eq 1
  return [ for $r in $refs[] return $r.k + $w[1].v ]
]
