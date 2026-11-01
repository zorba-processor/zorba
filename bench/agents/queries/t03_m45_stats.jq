(: Ground truth of t03_m45_stats (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $mags :=
  for $f in jn:members(
    dir:open($dir:root, "earthquakes/usgs_m2.5_week.geojson")("content")("features"))
  let $m := $f("properties")("mag")
  where $m ge 4.5
  return $m
return {
  "count" : count($mags),
  "mean_mag" : round-half-to-even(avg($mags), 2)
}
