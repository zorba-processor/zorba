jsoniq version "1.0";
let $ws := (
  for tumbling window $w in (for $i in 1 to 10000 return {"i":$i})
    start $s at $p previous $q next $n when $p mod 10 eq 1
  return { "a": $s.i, "q": $q.i, "sum": sum($w.i), "n": $n.i })
return [ count($ws), sum($ws.sum), $ws[1], $ws[last()] ]
