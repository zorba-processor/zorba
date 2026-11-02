(: A key of type xs:float arriving after many large integers must find the integer
   it equals, and integers arriving after it must find it (fn:distinct-values,
   F&O 3.0: no two items of the result compare equal; XQuery 3.0 3.10.7 likewise).
   16777216 + i for i in 1..5000: xs:float(16777216) eq 16777217 only. Several
   grouping keys are hashed together. :)
let $ints := for $i in 1 to 5000 return 16777216 + $i
let $f := xs:float(16777216)
return (
  count(distinct-values(($ints, $f))),
  count(distinct-values(($f, $ints))),
  count(for $x in ($ints, $f) group by $k := $x return $k),
  count(for $x in ($f, $ints) group by $k := $x return $k),
  count(for $x in ($ints, $f) group by $s := "s", $k := $x return $k),
  count(for $x in ($f, $ints) group by $k := $x, $s := "s" return $k),
  count(distinct-values(($ints, $f, "a", xs:untypedAtomic("a"), 1.5, 1.5e0, xs:float(1.5), xs:float("NaN"), xs:double("NaN"))))
)
