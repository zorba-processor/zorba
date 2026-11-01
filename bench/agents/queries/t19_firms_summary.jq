(: Ground truth of t19_firms_summary (see tasks/tasks.json).
 : csv:parse() keeps the field names of its first input when the same call
 : site parses a second file (e.g. dir:documents($dir:root, "csv") in a
 : FLWOR), so each fire file is parsed by its own csv:parse() call. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
import module namespace file = "http://expath.org/ns/file";
import module namespace csv = "http://zorba.io/modules/json-csv";

let $fires := (
  { "path" : "fires/firms_modis_americas_24h.csv",
    "rows" : [ csv:parse(file:read-text(
      concat($dir:root, "/fires/firms_modis_americas_24h.csv"))) ] },
  { "path" : "fires/firms_modis_asia_pacific_24h.csv",
    "rows" : [ csv:parse(file:read-text(
      concat($dir:root, "/fires/firms_modis_asia_pacific_24h.csv"))) ] })
return {
  "files" : [
    for $d in $fires
    let $rows := jn:members($d("rows"))
    order by $d("path")
    return {
      "file" : $d("path"),
      "detections" : count($rows),
      "high_confidence" : count(
        for $r in $rows where xs:integer($r("confidence")) ge 80 return $r),
      "total_frp" : round-half-to-even(
        sum(for $r in $rows return xs:double($r("frp"))), 1)
    }
  ]
}
