(: Ground truth of t01_largest_quake (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
(: dir:format() does not know the .geojson extension, so dir:open() reports
 : the GeoJSON file as unsupported: it is read and parsed explicitly. :)
import module namespace file = "http://expath.org/ns/file";

let $features := jn:members(
  jn:parse-json(file:read-text(
    concat($dir:root, "/earthquakes/usgs_m2.5_week.geojson")))("features"))
let $max := max(for $f in $features return $f("properties")("mag"))
for $f in $features
where $f("properties")("mag") eq $max
return {
  "id" : $f("id"),
  "mag" : $max,
  "place" : $f("properties")("place")
}
