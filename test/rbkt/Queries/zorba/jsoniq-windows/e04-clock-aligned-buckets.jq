jsoniq version "1.0";
(: XQuery 3.0 B.2 op:divide-dayTimeDuration-by-dayTimeDuration gives the number of
   15-minute buckets since midnight; a window per bucket that has records. :)
declare function local:b($x) {
  xs:integer(floor((xs:dateTime($x.ts) - xs:dateTime("2026-01-01T00:00:00Z"))
                   div xs:dayTimeDuration("PT15M")))
};
let $a := [ {"id":1,"ts":"2026-01-01T10:01:00Z"}, {"id":2,"ts":"2026-01-01T10:14:00Z"},
            {"id":3,"ts":"2026-01-01T10:15:00Z"}, {"id":4,"ts":"2026-01-01T10:29:00Z"},
            {"id":5,"ts":"2026-01-01T10:46:00Z"}, {"id":6,"ts":"2026-01-01T11:00:00Z"} ]
return [
  for tumbling window $w in $a[]
    start $s when true
    end next $n when empty($n) or local:b($n) ne local:b($s)
  return { "b": local:b($s), "ids": [ $w.id ] }
]
