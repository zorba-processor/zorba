jsoniq version "1.0";
(: A window of up to 10 minutes from each record. The window of a record ends
   before the first later record that is more than 10 minutes away. :)
declare function local:t($x) { xs:dateTime($x.ts) };
let $a := [ {"v":0,"ts":"2026-01-01T09:00:00Z"}, {"v":4,"ts":"2026-01-01T09:04:00Z"},
            {"v":9,"ts":"2026-01-01T09:09:00Z"}, {"v":12,"ts":"2026-01-01T09:12:00Z"},
            {"v":25,"ts":"2026-01-01T09:25:00Z"} ]
return [
  for sliding window $w in $a[]
    start $s when true
    only end next $n when empty($n) or local:t($n) - local:t($s) gt xs:dayTimeDuration("PT10M")
  return { "from": $s.v, "n": count($w), "sum": sum($w.v) }
]
