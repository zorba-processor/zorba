(: Ground truth of t15_city_nearest_quake (see tasks/tasks.json). :)
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
return {
  "cities" : [
    for $city in jn:members(dir:open($dir:root, "weather/cities.csv")("content"))
    let $la := xs:double($city("latitude"))
    let $lo := xs:double($city("longitude"))
    let $nearest := (
      for $q in $quakes
      let $d := local:km($la, $lo, $q("lat"), $q("lon"))
      order by $d
      return { "quake_id" : $q("id"), "distance_km" : $d })[1]
    let $name := string($city("city"))
    order by $name
    return {
      "city" : $name,
      "quake_id" : $nearest("quake_id"),
      "distance_km" : round-half-to-even($nearest("distance_km"), 1)
    }
  ]
}
