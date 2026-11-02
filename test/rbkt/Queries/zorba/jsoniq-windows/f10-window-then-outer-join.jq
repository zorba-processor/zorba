jsoniq version "1.0";
(: The same with `allowing empty` (XQuery 3.0 3.10.2): the window without a match stays. :)
let $meta := [ {"id":"a","name":"A"}, {"id":"b","name":"B"} ]
let $data := [ {"id":"a","v":1}, {"id":"a","v":2}, {"id":"b","v":3}, {"id":"c","v":4} ]
return [
  for tumbling window $w in $data[]
    start $s previous $p when empty($p) or $s.id ne $p.id
  for $m allowing empty in $meta[][$$.id eq $w[1].id]
  return { "name": $m.name, "sum": sum($w.v) }
]
