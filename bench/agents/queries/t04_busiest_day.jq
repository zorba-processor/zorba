(: Ground truth of t04_busiest_day (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $days :=
  for $r in jn:members(
    dir:open($dir:root, "earthquakes/usgs_m2.5_week.csv")("content"))
  group by $day := substring(string($r("time")), 1, 10)
  order by count($r) descending, $day
  return { "date" : $day, "count" : count($r) }
return $days[1]
