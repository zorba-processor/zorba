(: Ground truth of t13_cyclone_categories (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
declare namespace gdacs = "http://www.gdacs.org";

let $page :=
  dir:open($dir:root, "reference/nhc_saffir_simpson_scale.html")("content")
let $bounds :=
  for $tr in ($page//table)[1]//tr
  let $cat := normalize-space(string($tr/td[1]))
  where matches($cat, "^[1-5]")
  let $kmh :=
    for $t in $tr/td[2]//text()
    where contains($t, "km/h")
    return xs:integer(replace(normalize-space($t), "^([0-9]+).*$", "$1"))
  return { "category" : xs:integer(substring($cat, 1, 1)), "low" : $kmh[1] }
return {
  "cyclones" : [
    for $i in dir:open($dir:root, "alerts/gdacs_rss.xml")("content")//item
    where string($i/gdacs:eventtype) eq "TC"
    let $wind := xs:double($i/gdacs:severity/@value)
    let $cat := max((0, for $b in $bounds
                        where $wind ge $b("low")
                        return $b("category")))
    let $id := string($i/gdacs:eventid)
    order by $id
    return {
      "eventid" : $id,
      "name" : string($i/gdacs:eventname),
      "wind_kmh" : round-half-to-even($wind, 1),
      "category" : $cat
    }
  ]
}
