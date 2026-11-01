(: Grouping keys that are equal under value comparison fall in one group. :)
declare function local:groups($keys as xs:anyAtomicType*) as xs:integer* {
  for $k in $keys
  group by $g := $k
  order by count($k)
  return count($k)
};

(
  (: 2^53+1 and the double 2^53 are equal; 5 is not :)
  local:groups((9007199254740993, 9.007199254740992E15, 5)),
  "|",
  local:groups((xs:decimal("18446744073709551615"), 1.8446744073709552E19)),
  "|",
  local:groups((xs:double("NaN"), xs:float("NaN"), 1)),
  "|",
  (: NaN in the first key must not hide a difference in the second; the two
     integers share their low 32 bits :)
  for $p in (1, 4294967297)
  let $n := xs:double("NaN")
  group by $n, $p
  order by $p
  return count($p)
)
