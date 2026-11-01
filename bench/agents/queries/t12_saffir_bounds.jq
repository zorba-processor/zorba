(: Ground truth of t12_saffir_bounds (see tasks/tasks.json).
 : dir:open() reports HTML files as unsupported (the directory module's
 : HOOK(parse-html) is not wired to x:parse-html yet), so the page is read
 : with the file module and parsed with x:parse-html(). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace file = "http://expath.org/ns/file";
import module namespace x = "http://zorba.io/modules/xml";

let $page := x:parse-html(file:read-text(
  concat($dir:root, "/reference/nhc_saffir_simpson_scale.html")))
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
