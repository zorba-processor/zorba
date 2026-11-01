(: Ground truth of t18_hot_cities_near_alerts (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
declare namespace math = "http://www.w3.org/2005/xpath-functions/math";
declare namespace georss = "http://www.georss.org/georss";

declare function local:km($la1 as xs:double, $lo1 as xs:double,
                          $la2 as xs:double, $lo2 as xs:double) as xs:double
{
  let $r := math:pi() div 180
  let $h := math:pow(math:sin(($la2 - $la1) * $r div 2), 2) +
            math:cos($la1 * $r) * math:cos($la2 * $r) *
            math:pow(math:sin(($lo2 - $lo1) * $r div 2), 2)
  return 2 * 6371.0 * math:asin(math:sqrt($h))
};

let $points :=
  for $i in dir:open($dir:root, "alerts/gdacs_rss.xml")("content")//item
  let $p := tokenize(normalize-space($i/georss:point), " ")
  return [ xs:double($p[1]), xs:double($p[2]) ]
return {
  "cities" : [
    for $city in jn:members(dir:open($dir:root, "weather/cities.csv")("content"))
    let $w := dir:open($dir:root, string($city("file")))
    where exists($w("content"))
    where $w("content")("daily")("temperature_2m_max")[[1]] ge 30
    let $la := xs:double($city("latitude"))
    let $lo := xs:double($city("longitude"))
    let $n := count(
      for $p in $points
      where local:km($la, $lo, $p[[1]], $p[[2]]) le 500
      return $p)
    where $n gt 0
    let $name := string($city("city"))
    order by $name
    return { "city" : $name, "alerts_within_500km" : $n }
  ]
}
