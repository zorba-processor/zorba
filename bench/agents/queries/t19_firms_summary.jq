(: Ground truth of t19_firms_summary (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $fires :=
  for $d in dir:documents($dir:root, "csv")
  where starts-with($d("path"), "fires/")
  return { "path" : $d("path"), "rows" : $d("content") }
return {
  "files" : [
    for $d in $fires
    let $rows := jn:members($d("rows"))
    order by $d("path")
    return {
      "file" : $d("path"),
      "detections" : count($rows),
      "high_confidence" : count(
        for $r in $rows where xs:integer($r("confidence")) ge 80 return $r),
      "total_frp" : round-half-to-even(
        sum(for $r in $rows return xs:double($r("frp"))), 1)
    }
  ]
}
