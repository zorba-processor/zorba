(: Equal numeric values hash and compare equal whatever their size and
   however they were computed: distinct-values and group by. :)
let $d := ( 1.0, 1, 1.50, 1.5, 0.5 + 1, xs:decimal( "1.5" ), -0.0, 0, 0.000,
            9223372036854775807, 9223372036854775808.0,
            9223372036854775807.5,
            123456789012345678901234567890.123456789,
            123456789012345678901234567890.1234567890 )
return string-join( (
  string( count( distinct-values( $d ) ) ),
  for $x in $d
  group by $k := $x
  order by $k
  return string( count( $x ) ) ), " " )
