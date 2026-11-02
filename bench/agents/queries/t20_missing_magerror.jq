(: Ground truth of t20_missing_magerror (see tasks/tasks.json). csv:parse()
 : (used by the directory module) turns an empty value into null. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $ids :=
  for $r in jn:members(
    dir:open($dir:root, "earthquakes/usgs_m2.5_week.csv")("content"))
  let $e := $r("magError")
  where empty($e) or $e eq null
  order by string($r("id"))
  return string($r("id"))
return { "count" : count($ids), "ids" : [ $ids ] }
