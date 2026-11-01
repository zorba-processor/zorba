(: Ground truth of t02_magtype_counts (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $features := jn:members(
  dir:open($dir:root, "earthquakes/usgs_m2.5_week.geojson")("content")("features"))
return {
  "counts" : [
    for $f in $features
    group by $t := $f("properties")("magType")
    order by $t
    return { "magType" : $t, "count" : count($f) }
  ]
}
