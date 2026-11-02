jsoniq version "1.0";
(: XQuery 3.0 3.10.7: after group by, $w is the concatenation of its values in tuple order. :)
let $a := [ {"cat":"a","v":1}, {"cat":"a","v":2}, {"cat":"b","v":3}, {"cat":"a","v":4} ]
return [
  for tumbling window $w in $a[]
    start $s previous $p when empty($p) or $s.cat ne $p.cat
  group by $k := $w[1].cat
  order by $k
  return { "k": $k, "vals": [ $w.v ] }
]
