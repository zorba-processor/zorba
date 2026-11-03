(: fn:distinct-values (F&O 3.0) and group by (XQuery 3.0 3.10.7): values above 2^63-1
   equal by promotion merge, values that differ stay apart. index-of and deep-equal
   use the same equality; order by and min/max the same order. :)
let $m := xs:unsignedLong("18446744073709551615")
let $b := xs:unsignedLong("9223372036854775808")
let $c := xs:unsignedLong("9223372036854775809")
return (
  count(distinct-values(($m, 18446744073709551615.0))),
  count(distinct-values((18446744073709551615.0, $m))),
  count(distinct-values(($m, $b, $c, $m, $b, $c))),
  count(distinct-values(($b, $c, 9.223372036854775808E18))) ge 1,
  count(distinct-values(($m, 1.8446744073709551616E19))),
  count(distinct-values(($m, xs:float("1.8446744073709551616E19")))),
  count(distinct-values(($m, 9223372036854775807, -1))),
  count(for $x in ($m, 18446744073709551615.0) group by $k := $x return $k),
  count(for $x in ($m, $b, $c, $m, $b, $c) group by $k := $x return $k),
  count(for $x in ($m, 1.8446744073709551616E19) group by $k := $x return $k),
  count(for $x in ($m, xs:float("1.8446744073709551616E19")) group by $k := $x return $k),
  count(for $x in (xs:unsignedLong("1"), $m, $m) group by $k := $x return $k),
  index-of(($b, $m, 18446744073709551615.0), $m),
  deep-equal(($m, $b), (18446744073709551615.0, 9223372036854775808.0)),
  string-join(for $x in ($m, -1, $b, 5, $c) order by $x return string($x), ","),
  string(max(($b, $m, 5))),
  string(min(($b, $m, 5.5)))
)
