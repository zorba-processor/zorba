(: Moving average over 3 readings. A missing or null value does not count. :)
let $readings := [
  { "t": "2026-03-01T10:00:00Z", "temp": 21.5 },
  { "t": "2026-03-01T10:05:00Z", "temp": 22.5 },
  { "t": "2026-03-01T10:10:00Z" },
  { "t": "2026-03-01T10:15:00Z", "temp": null },
  { "t": "2026-03-01T10:20:00Z", "temp": 24.5 }
]
return [
  for sliding window $w in $readings[]
    start at $s when true
    only end at $e when $e - $s eq 2
  return {
    "from": $w[1].t,
    "avg": avg($w.temp[$$ instance of decimal])
  }
]
