(: Ground truth of t01_largest_quake (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $features := jn:members(
  dir:open($dir:root, "earthquakes/usgs_m2.5_week.geojson")("content")("features"))
let $max := max(for $f in $features return $f("properties")("mag"))
for $f in $features
where $f("properties")("mag") eq $max
return {
  "id" : $f("id"),
  "mag" : $max,
  "place" : $f("properties")("place")
}
