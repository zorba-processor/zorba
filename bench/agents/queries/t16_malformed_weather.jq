(: Ground truth of t16_malformed_weather (see tasks/tasks.json). A file that
 : cannot be parsed comes back from dir:documents() as a record with an
 : "error" member instead of "content". :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $cities := jn:members(dir:open($dir:root, "weather/cities.csv")("content"))
let $docs :=
  for $d in dir:documents($dir:root, "json")
  where starts-with($d("path"), "weather/")
  return $d
let $ranked :=
  for $d in $docs
  where exists($d("content"))
  let $t := $d("content")("current")("temperature_2m")
  order by $t descending
  return {
    "city" : string(
      (for $c in $cities where $c("file") eq $d("path") return $c("city"))[1]),
    "temperature_c" : $t
  }
return {
  "city" : $ranked[1]("city"),
  "temperature_c" : $ranked[1]("temperature_c"),
  "unreadable_files" : [
    for $d in $docs
    where exists($d("error"))
    order by $d("path")
    return $d("path")
  ]
}
