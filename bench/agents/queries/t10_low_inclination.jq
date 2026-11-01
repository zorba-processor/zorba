(: Ground truth of t10_low_inclination (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";

let $omm := dir:open($dir:root, "satellites/celestrak_stations_omm.xml")("content")
return {
  "object_names" : [
    for $s in $omm//segment
    where xs:double($s/data/meanElements/INCLINATION) lt 45
    let $name := string($s/metadata/OBJECT_NAME)
    order by $name
    return $name
  ]
}
