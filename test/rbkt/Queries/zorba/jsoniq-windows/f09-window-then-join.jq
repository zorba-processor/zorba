jsoniq version "1.0";
(: A window followed by a join on a value of the window. "c" has no match. :)
let $meta := [ {"id":"a","name":"A"}, {"id":"b","name":"B"} ]
let $data := [ {"id":"a","v":1}, {"id":"a","v":2}, {"id":"b","v":3}, {"id":"c","v":4} ]
return [
  for tumbling window $w in $data[]
    start $s previous $p when empty($p) or $s.id ne $p.id
  for $m in $meta[]
  where $m.id eq $w[1].id
  return { "name": $m.name, "sum": sum($w.v) }
]
