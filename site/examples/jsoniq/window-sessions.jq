(: Sessions: a new one after a gap above 5 minutes. Timestamps are strings in
   mixed time zones; cast to xs:dateTime they compare as instants. :)
declare function local:t($e) { xs:dateTime($e.ts) };

let $events := [
  { "ts": "2026-03-01T09:00:00Z", "page": "home" },
  { "ts": "2026-03-01T10:02:00+01:00", "page": "search" },
  { "ts": "2026-03-01T09:30:00Z", "page": "home" },
  { "ts": "2026-03-01T09:31:00Z" }
]
return [
  for tumbling window $w in $events[]
    start $s previous $p
      when empty($p) or local:t($s) - local:t($p) gt xs:dayTimeDuration("PT5M")
  return {
    "start": $s.ts,
    "events": count($w),
    "pages": [ $w.page ]
  }
]
