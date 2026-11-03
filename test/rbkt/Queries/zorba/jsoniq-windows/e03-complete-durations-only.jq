jsoniq version "1.0";
(: `only end`: the last period has no later record, so it may be incomplete and is dropped. :)
declare function local:t($x) { xs:dateTime($x.ts) };
let $a := [ {"id":1,"ts":"2026-01-01T10:00:00Z"},
            {"id":2,"ts":"2026-01-01T10:30:00Z"},
            {"id":3,"ts":"2026-01-01T10:45:00Z"},
            {"id":4,"ts":"2026-01-01T11:10:00Z"} ]
return [
  for tumbling window $w in $a[]
    start $s when true
    only end next $n when exists($n) and local:t($n) - local:t($s) ge xs:dayTimeDuration("PT30M")
  return [ $w.id ]
]
