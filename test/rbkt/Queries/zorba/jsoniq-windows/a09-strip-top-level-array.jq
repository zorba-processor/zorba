jsoniq version "1.0";
(: Input: parse-json with jsoniq-strip-top-level-array returns the members directly. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
[
  for tumbling window $w in
      parse-json(file:read-text($rbktPath || "/Queries/zorba/jsoniq-windows/data/series.json"),
                 { "jsoniq-strip-top-level-array": true })
    start at $s when true
    end at $e when $e - $s eq 3
  return [ $w.id ]
]
