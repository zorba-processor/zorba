jsoniq version "1.0";
(: `ne` is empty when a side is missing, so a missing state does not break a run;
   fn:deep-equal tells empty, null and a value apart (F&O 3.0 15.3.1, null equals
   only null: JSONiq extension 6.16). :)
let $a := [ {"i":1,"state":"a"}, {"i":2}, {"i":3,"state":"a"},
            {"i":4,"state":null}, {"i":5,"state":null}, {"i":6,"state":"b"} ]
return [
  [ for tumbling window $w in $a[]
      start $s previous $p when empty($p) or $s.state ne $p.state
    return [ $w.i ] ],
  [ for tumbling window $w in $a[]
      start $s previous $p when empty($p) or not(deep-equal($s.state, $p.state))
    return [ $w.i ] ]
]
