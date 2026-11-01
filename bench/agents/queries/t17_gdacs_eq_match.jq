(: Ground truth of t17_gdacs_eq_match (see tasks/tasks.json). GDACS dates are
 : RFC 822 ("Thu, 29 Oct 2026 07:49:50 GMT"); USGS times are milliseconds
 : since the epoch. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
declare namespace math = "http://www.w3.org/2005/xpath-functions/math";
declare namespace gdacs = "http://www.gdacs.org";
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

(: Seconds since the epoch of an RFC 822 date in GMT. :)
declare function local:rfc822($s as xs:string) as xs:decimal
{
  let $t := tokenize(normalize-space($s), " ")
  let $months := ("Jan", "Feb", "Mar", "Apr", "May", "Jun",
                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec")
  let $m := index-of($months, $t[3])
  let $dt := xs:dateTime(concat($t[4], "-",
    if ($m lt 10) then "0" else "", $m, "-",
    if (string-length($t[2]) eq 1) then "0" else "", $t[2], "T", $t[5], "Z"))
  return ($dt - xs:dateTime("1970-01-01T00:00:00Z"))
         div xs:dayTimeDuration("PT1S")
};

let $quakes :=
  for $f in jn:members(
    dir:open($dir:root, "earthquakes/usgs_m2.5_week.geojson")("content")("features"))
  where $f("properties")("mag") ge 4.5
  let $c := $f("geometry")("coordinates")
  return { "id" : $f("id"), "lat" : xs:double($c[[2]]),
           "lon" : xs:double($c[[1]]),
           "t" : $f("properties")("time") div 1000 }
return {
  "matches" : [
    for $i in dir:open($dir:root, "alerts/gdacs_rss.xml")("content")//item
    where string($i/gdacs:eventtype) eq "EQ"
    let $p := tokenize(normalize-space($i/georss:point), " ")
    let $la := xs:double($p[1])
    let $lo := xs:double($p[2])
    let $t := local:rfc822(string($i/gdacs:fromdate))
    let $match := (
      for $q in $quakes
      where abs($q("t") - $t) le 120
      let $d := local:km($la, $lo, $q("lat"), $q("lon"))
      where $d le 50
      order by $d
      return $q("id"))[1]
    let $id := string($i/gdacs:eventid)
    order by $id
    return { "eventid" : $id, "usgs_id" : if (exists($match)) then $match else null }
  ]
}
