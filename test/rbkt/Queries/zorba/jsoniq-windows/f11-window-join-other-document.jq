jsoniq version "1.0";
(: The join with another document read from a file. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
let $dir := $rbktPath || "/Queries/zorba/jsoniq-windows/data/"
let $lines := parse-json(file:read-text($dir || "lines.jsonl"))
return [
  for tumbling window $w in parse-json(file:read-text($dir || "series.json"))[]
    start at $s when true
    only end at $e when $e - $s eq 2
  for $m in $lines
  where $m.id eq $w[1].id
  return { "state": $m.state, "sum": sum($w.v) }
]
