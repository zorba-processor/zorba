(: Ground truth of t08_deepest_quakeml (see tasks/tasks.json). QuakeML
 : depths are in metres. :)
import module namespace dir = "http://zorba.io/modules/directory";
declare namespace q = "http://quakeml.org/xmlns/bed/1.2";
declare namespace catalog = "http://anss.org/xmlns/catalog/0.1";

let $qml := dir:open($dir:root, "earthquakes/usgs_m4.5_week_quakeml.xml")("content")
let $events :=
  for $e in $qml//q:event
  let $po := string($e/q:preferredOriginID)
  let $depth := xs:double($e/q:origin[@publicID eq $po]/q:depth/q:value)
  order by $depth descending
  return {
    "id" : concat($e/@catalog:datasource, $e/@catalog:eventid),
    "depth_km" : $depth div 1000
  }
return $events[1]
