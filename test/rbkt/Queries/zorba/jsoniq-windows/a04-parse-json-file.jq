jsoniq version "1.0";
(: Input: a JSON file read with the file module. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
[
  for sliding window $w in
      parse-json(file:read-text($rbktPath || "/Queries/zorba/jsoniq-windows/data/series.json"))[]
    start at $s when true
    only end at $e when $e - $s eq 2
  return avg($w.v)
]
