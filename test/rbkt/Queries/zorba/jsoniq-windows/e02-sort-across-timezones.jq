jsoniq version "1.0";
declare function local:t($x) { xs:dateTime($x.ts) };
let $a := [ {"id":4,"ts":"2026-01-01T11:10:00Z"},
            {"id":3,"ts":"2026-01-01T05:45:00-05:00"},
            {"id":1,"ts":"2026-01-01T10:00:00Z"},
            {"id":2,"ts":"2026-01-01T11:30:00+01:00"} ]
return [
  for tumbling window $w in (for $x in $a[] order by local:t($x) return $x)
    start at $s when true
    end at $e when $e - $s eq 1
  return [ $w.id ]
]
