jsoniq version "1.0";
(: Runs of at least 2 samples: a `where` clause after the window. :)
let $a := [ {"state":"a"}, {"state":"b"}, {"state":"b"}, {"state":"a"}, {"state":"a"}, {"state":"a"}, {"state":"c"} ]
return [
  for tumbling window $w in $a[]
    start $s previous $p when empty($p) or $s.state ne $p.state
  where count($w) ge 2
  return { "state": $s.state, "n": count($w) }
]
