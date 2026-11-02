(: Ground truth of t12_saffir_bounds (see tasks/tasks.json). :)
import module namespace dir = "http://zorba.io/modules/directory";

let $page :=
  dir:open($dir:root, "reference/nhc_saffir_simpson_scale.html")("content")
let $table := ($page//table)[1]
return {
  "lower_bounds_kmh" : [
    for $tr in $table//tr
    let $cat := normalize-space(string($tr/td[1]))
    where matches($cat, "^[1-5]")
    let $kmh :=
      for $t in $tr/td[2]//text()
      where contains($t, "km/h")
      return xs:integer(replace(normalize-space($t), "^([0-9]+).*$", "$1"))
    order by xs:integer(substring($cat, 1, 1))
    return $kmh[1]
  ]
}
