(: Ground truth of t03_m45_stats (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
(: dir:format() does not know the .geojson extension, so dir:open() reports
 : the GeoJSON file as unsupported: it is read and parsed explicitly. :)
import module namespace file = "http://expath.org/ns/file";

let $mags :=
  for $f in jn:members(
    jn:parse-json(file:read-text(
    concat($dir:root, "/earthquakes/usgs_m2.5_week.geojson")))("features"))
  let $m := $f("properties")("mag")
  where $m ge 4.5
  return $m
return {
  "count" : count($mags),
  "mean_mag" : round-half-to-even(avg($mags), 2)
}
