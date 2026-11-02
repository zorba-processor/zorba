(: Ground truth of t06_mag_revisions (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $g := jn:members(
  dir:open($dir:root, "earthquakes/usgs_m2.5_week.geojson")("content")("features"))
return {
  "events" : [
    for $r in jn:members(
      dir:open($dir:root, "earthquakes/usgs_m2.5_week.csv")("content"))
    let $id := string($r("id"))
    let $cm := xs:double($r("mag"))
    for $f in $g
    where string($f("id")) eq $id
    let $gm := xs:double($f("properties")("mag"))
    where abs($cm - $gm) gt 0.001
    order by $id
    return { "id" : $id, "geojson_mag" : $gm, "csv_mag" : $cm }
  ]
}
