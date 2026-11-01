(: Ground truth of t05_geojson_vs_csv (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
(: dir:format() does not know the .geojson extension, so dir:open() reports
 : the GeoJSON file as unsupported: it is read and parsed explicitly. :)
import module namespace file = "http://expath.org/ns/file";

let $g :=
  for $f in jn:members(
    jn:parse-json(file:read-text(
    concat($dir:root, "/earthquakes/usgs_m2.5_week.geojson")))("features"))
  return string($f("id"))
let $c :=
  for $r in jn:members(
    dir:open($dir:root, "earthquakes/usgs_m2.5_week.csv")("content"))
  return string($r("id"))
return {
  "only_in_geojson" : [ for $i in $g where not($i = $c) order by $i return $i ],
  "only_in_csv" : [ for $i in $c where not($i = $g) order by $i return $i ]
}
