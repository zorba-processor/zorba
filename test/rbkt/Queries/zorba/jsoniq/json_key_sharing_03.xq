(: Large and small objects of one parse: key order and lookups. :)
import module namespace jn = "http://jsoniq.org/functions";

let $big := "{ " || string-join(for $i in 1 to 100 return '"k' || $i || '" : ' || $i, ", ") || " }"
let $a := jn:parse-json("[ " || $big || ", { ""k2"" : 0, ""k1"" : 1 }, " || $big || " ]")
return (
  string-join(jn:keys($a(1)), ",") eq string-join(for $i in 1 to 100 return "k" || $i, ","),
  $a(1)("k1"), $a(1)("k57"), $a(1)("k100"), empty($a(1)("k101")),
  string-join(jn:keys($a(2)), ","), $a(2)("k1"), $a(2)("k2"),
  every $i in 1 to 100 satisfies $a(3)("k" || $i) eq $i
)
