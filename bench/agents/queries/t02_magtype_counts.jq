(: Ground truth of t02_magtype_counts (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
(: dir:format() does not know the .geojson extension, so dir:open() reports
 : the GeoJSON file as unsupported: it is read and parsed explicitly. :)
import module namespace file = "http://expath.org/ns/file";

let $features := jn:members(
  jn:parse-json(file:read-text(
    concat($dir:root, "/earthquakes/usgs_m2.5_week.geojson")))("features"))
return {
  "counts" : [
    for $f in $features
    group by $t := $f("properties")("magType")
    order by $t
    return { "magType" : $t, "count" : count($f) }
  ]
}
