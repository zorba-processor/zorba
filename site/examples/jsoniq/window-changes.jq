(: Runs of the same state: `previous` detects each change. :)
let $log := [
  { "t": 1, "state": "ok" },
  { "t": 2, "state": "ok" },
  { "t": 3, "state": "fail", "why": "disk" },
  { "t": 4, "state": "fail" },
  { "t": 5, "state": "ok" }
]
return [
  for tumbling window $w in $log[]
    start $s previous $p when empty($p) or $s.state ne $p.state
  return {
    "state": $s.state,
    "from": $s.t,
    "samples": count($w)
  }
]
