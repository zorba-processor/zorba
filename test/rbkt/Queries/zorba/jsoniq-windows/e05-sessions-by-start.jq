jsoniq version "1.0";
(: Sessions split by a gap above 5 minutes, written with the start condition. :)
declare function local:t($x) { xs:dateTime($x.ts) };
let $a := [ {"ts":"2026-01-01T09:00:00Z"}, {"ts":"2026-01-01T09:01:00Z"}, {"ts":"2026-01-01T09:03:00Z"},
            {"ts":"2026-01-01T09:10:00Z"}, {"ts":"2026-01-01T09:11:00Z"}, {"ts":"2026-01-01T09:30:00Z"} ]
return [
  for tumbling window $w in $a[]
    start $s previous $p when empty($p) or local:t($s) - local:t($p) gt xs:dayTimeDuration("PT5M")
  return { "from": $w[1].ts, "to": $w[last()].ts, "n": count($w),
           "dur": string(local:t($w[last()]) - local:t($w[1])) }
]
