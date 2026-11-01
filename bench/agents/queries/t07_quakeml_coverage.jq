(: Ground truth of t07_quakeml_coverage (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
declare namespace q = "http://quakeml.org/xmlns/bed/1.2";
declare namespace catalog = "http://anss.org/xmlns/catalog/0.1";

let $qml := dir:open($dir:root, "earthquakes/usgs_m4.5_week_quakeml.xml")("content")
let $q :=
  for $e in $qml//q:event
  return concat($e/@catalog:datasource, $e/@catalog:eventid)
let $big :=
  for $f in jn:members(
    dir:open($dir:root, "earthquakes/usgs_m2.5_week.geojson")("content")("features"))
  where $f("properties")("mag") ge 4.5
  return string($f("id"))
return {
  "geojson_m45_not_in_quakeml" :
    [ for $i in $big where not($i = $q) order by $i return $i ],
  "quakeml_not_in_geojson_m45" :
    [ for $i in $q where not($i = $big) order by $i return $i ]
}
