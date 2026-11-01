(: Ground truth of t14_fires_near_quakes (see tasks/tasks.json). The latitude
 : prefilter (100 km < 0.9 degrees of latitude) only skips pairs that cannot
 : be within 100 km. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
declare namespace math = "http://www.w3.org/2005/xpath-functions/math";

declare function local:km($la1 as xs:double, $lo1 as xs:double,
                          $la2 as xs:double, $lo2 as xs:double) as xs:double
{
  let $r := math:pi() div 180
  let $h := math:pow(math:sin(($la2 - $la1) * $r div 2), 2) +
            math:cos($la1 * $r) * math:cos($la2 * $r) *
            math:pow(math:sin(($lo2 - $lo1) * $r div 2), 2)
  return 2 * 6371.0 * math:asin(math:sqrt($h))
};

let $quakes :=
  for $f in jn:members(
    dir:open($dir:root, "earthquakes/usgs_m2.5_week.geojson")("content")("features"))
  where $f("properties")("mag") ge 4.5
  let $c := $f("geometry")("coordinates")
  return { "id" : $f("id"), "lat" : xs:double($c[[2]]),
           "lon" : xs:double($c[[1]]) }
let $fires :=
  for $d in dir:documents($dir:root, "csv")
  where starts-with($d("path"), "fires/")
  return { "path" : $d("path"), "rows" : $d("content") }
let $pairs :=
  for $d in $fires
  for $r at $n in jn:members($d("rows"))
  let $la := xs:double($r("latitude"))
  let $lo := xs:double($r("longitude"))
  for $q in $quakes
  where abs($la - $q("lat")) le 1.0
    and local:km($la, $lo, $q("lat"), $q("lon")) le 100
  return { "fire" : concat($d("path"), "#", $n), "quake" : $q("id") }
return {
  "fire_detections" : count(distinct-values(for $p in $pairs return $p("fire"))),
  "quake_ids" : [
    for $id in distinct-values(for $p in $pairs return $p("quake"))
    order by $id
    return $id
  ]
}
