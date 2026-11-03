jsoniq version "1.0";
(: A record without a timestamp: the gap test is empty, hence false, on both sides
   of it, so it stays in the session. Removing such records first gives a real gap. :)
declare function local:t($x) { xs:dateTime($x.ts) };
let $a := [ {"i":1,"ts":"2026-01-01T09:00:00Z"}, {"i":2}, {"i":3,"ts":"2026-01-01T09:20:00Z"},
            {"i":4,"ts":"2026-01-01T09:21:00Z"}, {"i":5,"ts":"2026-01-01T09:40:00Z"} ]
return [
  [ for tumbling window $w in $a[]
      start $s previous $p when empty($p) or local:t($s) - local:t($p) gt xs:dayTimeDuration("PT5M")
    return [ $w.i ] ],
  [ for tumbling window $w in $a[][exists($$.ts)]
      start $s previous $p when empty($p) or local:t($s) - local:t($p) gt xs:dayTimeDuration("PT5M")
    return [ $w.i ] ]
]
