jsoniq version "1.0";
(: Runs of the same value, from the change detected with `previous`. :)
let $a := [ {"state":"a"}, {"state":"a"}, {"state":"b"}, {"state":"b"}, {"state":"b"}, {"state":"a"} ]
return [
  for tumbling window $w in $a[]
    start $s previous $p when empty($p) or $s.state ne $p.state
  return { "state": $s.state, "n": count($w) }
]
