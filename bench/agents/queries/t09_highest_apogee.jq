(: Ground truth of t09_highest_apogee (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
declare namespace math = "http://www.w3.org/2005/xpath-functions/math";

declare function local:apogee($mm as xs:double, $ecc as xs:double)
  as xs:double
{
  let $n := $mm * 2 * math:pi() div 86400
  let $a := math:pow(398600.4418 div ($n * $n), 1 div 3)
  return $a * (1 + $ecc) - 6378.137
};

let $ranked :=
  for $r in jn:members(
    dir:open($dir:root, "satellites/celestrak_stations.csv")("content"))
  let $alt := local:apogee(xs:double($r("MEAN_MOTION")),
                           xs:double($r("ECCENTRICITY")))
  order by $alt descending
  return {
    "norad_cat_id" : xs:integer($r("NORAD_CAT_ID")),
    "object_name" : string($r("OBJECT_NAME")),
    "apogee_km" : round-half-to-even($alt, 1)
  }
return $ranked[1]
