jsoniq version "1.0";
(: Input: a CSV read. The empty value is null (csv:parse missing-value default),
   so the numeric filter drops it from the sum. :)
import module namespace file = "http://expath.org/ns/file";
import module namespace csv = "http://zorba.io/modules/json-csv";
declare variable $rbktPath as string external;
[
  for tumbling window $w in
      csv:parse(file:read-text($rbktPath || "/Queries/zorba/jsoniq-windows/data/readings.csv"))
    start $s previous $p when empty($p) or $s.sensor ne $p.sensor
  return { "sensor": $w[1].sensor, "n": count($w), "sum": sum($w.value[$$ instance of xs:decimal]) }
]
