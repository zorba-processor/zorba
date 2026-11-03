(: F&O 3.0 B.1 promotion: xs:unsignedLong against xs:double or xs:float compares
   after casting to the floating type. Above 2^53, values that round alike are
   equal. :)
let $m := xs:unsignedLong("18446744073709551615")
let $b := xs:unsignedLong("9223372036854775809")
return (
  $m eq 1.8446744073709551616E19, 1.8446744073709551616E19 eq $m,
  $m eq xs:float("1.8446744073709551616E19"), xs:float("1.8446744073709551616E19") eq $m,
  $b eq 9.223372036854775808E18, $b eq xs:float("9.223372036854775808E18"),
  $m gt 1.0E19, 1.0E19 lt $m, $m lt 1.0E20, 1.0E20 gt $m,
  $m gt xs:float("1E19"), $m lt xs:float("1E20"),
  $m lt xs:double("INF"), $m gt xs:double("-INF"), $m eq xs:double("NaN"), $m ne xs:float("NaN")
)
