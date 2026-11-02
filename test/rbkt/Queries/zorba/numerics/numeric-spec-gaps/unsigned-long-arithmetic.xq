(: F&O 3.0 4.2: arithmetic with an xs:decimal operand promotes the xs:unsignedLong to
   xs:decimal and is exact; with xs:double or xs:float, it casts. These do not depend
   on the range of xs:integer. :)
let $m := xs:unsignedLong("18446744073709551615")
return (
  $m + 0.5, 0.5 + $m, $m - 0.5, $m * 1.0, $m div 1.0, $m mod 7.0,
  $m + 1.0, -1.0 + $m,
  $m + 0e0 eq 1.8446744073709551616E19, $m * 1e0 eq 1.8446744073709551616E19,
  $m + xs:float(0) eq xs:float("1.8446744073709551616E19"),
  $m div 1e0 eq 1.8446744073709551616E19
)
