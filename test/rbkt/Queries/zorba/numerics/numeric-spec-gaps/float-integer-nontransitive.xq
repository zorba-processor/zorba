(: F&O 3.0 fn:distinct-values: "In the situation where the input contains three
   values A, B, and C such that A eq B, B eq C, but A ne C, then the number of
   items in the result of the function (as well as the choice of which items are
   returned) is implementation dependent, subject only to the constraints that
   (a) no two items in the result sequence compare equal to each other, and (b)
   every input item that does not appear in the result sequence compares equal to
   some item that does appear in the result sequence."
   XQuery 3.0 3.10.7 states the same constraints for group by.
   Each triple is checked in all six input orders:
   t1 16777216 eq f, 16777217 eq f, 16777216 ne 16777217;
   t2 2^60+2^36+1 casts to xs:float above and through xs:double below: the
      integer equals the xs:float and the xs:double, which differ;
   t3 the same with a decimal near a midpoint of two xs:float values near 1. :)
declare function local:spec($r as xs:anyAtomicType*, $input as xs:anyAtomicType*) as xs:boolean
{
  (every $i in 1 to count($r), $j in 1 to count($r)
   satisfies $i eq $j or $r[$i] ne $r[$j]) and
  (every $x in $input satisfies some $y in $r satisfies $x eq $y)
};
declare function local:check($a as xs:anyAtomicType, $b as xs:anyAtomicType, $c as xs:anyAtomicType) as xs:string
{
  string-join(
    for $p in ([$a,$b,$c], [$a,$c,$b], [$b,$a,$c], [$b,$c,$a], [$c,$a,$b], [$c,$b,$a])
    let $input := $p[]
    return concat(
      if (local:spec(distinct-values($input), $input)) then "d" else "D",
      if (local:spec(for $x in $input group by $k := $x return $k, $input)) then "g" else "G"),
    "")
};
let $x2 := 1152921573326323713
let $x3 := 1.0000000596046447753906250001
return (
  $x2 eq xs:float($x2), $x2 eq xs:double($x2), xs:float($x2) eq xs:double($x2),
  $x3 eq xs:float($x3), $x3 eq xs:double($x3), xs:float($x3) eq xs:double($x3),
  local:check(16777216, 16777217, xs:float(16777216)),
  local:check(-16777216, -16777217, xs:float(-16777216)),
  local:check($x2, xs:float($x2), xs:double($x2)),
  local:check($x3, xs:float($x3), xs:double($x3))
)
