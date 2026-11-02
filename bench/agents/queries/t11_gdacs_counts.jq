(: Ground truth of t11_gdacs_counts (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
declare namespace gdacs = "http://www.gdacs.org";

let $items := dir:open($dir:root, "alerts/gdacs_rss.xml")("content")//item
return {
  "by_type" : [
    for $i in $items
    group by $t := string($i/gdacs:eventtype)
    order by $t
    return { "eventtype" : $t, "count" : count($i) }
  ],
  "red_alerts" : count(
    for $i in $items where string($i/gdacs:alertlevel) eq "Red" return $i)
}
