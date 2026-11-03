jsoniq version "1.0";
(: Many windows: the variables stay right while the clause frees the items behind it.
   Sum of the first window 1..10 is 55; of the last one 9991..10000 is 99955. :)
let $ws := (
  for tumbling window $w in (for $i in 1 to 10000 return {"i":$i})
    start $s at $p previous $q when true
    end at $e next $n when $e - $p eq 9
  return { "a": $s.i, "q": $q.i, "sum": sum($w.i), "n": $n.i })
return [ count($ws), sum($ws.sum), $ws[1], $ws[last()] ]
