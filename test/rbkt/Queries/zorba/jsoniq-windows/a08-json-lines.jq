jsoniq version "1.0";
(: Input: JSON Lines, irregular records, runs of the same state. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
[
  for tumbling window $w in
      parse-json(file:read-text($rbktPath || "/Queries/zorba/jsoniq-windows/data/lines.jsonl"))
    start $s previous $p when empty($p) or $s.state ne $p.state
  return { "state": $w[1].state, "ids": [ $w.id ] }
]
