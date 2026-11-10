(: unordered-maps.xq map:get: "getting the entries is based on the "eq" comparison".
   eq does not compare an xs:date with an xs:dateTime or an xs:gYear, so such keys
   are not equal even when they denote the same instant. Many keys, so that dates
   and dateTimes meet in the same hash bucket. :)
import module namespace map = "http://zorba.io/modules/unordered-maps";
variable $r := ();
map:create("dt-facets", xs:QName("xs:anyAtomicType"));
for $i in 0 to 2999
return (
  map:insert("dt-facets", xs:date("2000-01-01Z") + xs:dayTimeDuration("P1D") * $i, "d"),
  map:insert("dt-facets",
             xs:dateTime("2000-01-01T00:00:00Z") + xs:dayTimeDuration("P1D") * $i, "dt")
);
map:create("date-only", xs:QName("xs:anyAtomicType"));
map:insert("date-only", xs:date("2013-01-01Z"), "d");
$r := (
  map:size("dt-facets"),
  map:get("dt-facets", xs:dateTime("2000-01-02T00:00:00Z")),
  map:get("dt-facets", xs:date("2000-01-02Z")),
  count(map:get("date-only", xs:dateTime("2013-01-01T00:00:00Z"))),
  count(map:get("date-only", xs:gYear("2013Z"))),
  map:get("date-only", xs:date("2013-01-01Z"))
);
map:drop("dt-facets");
map:drop("date-only");
$r
