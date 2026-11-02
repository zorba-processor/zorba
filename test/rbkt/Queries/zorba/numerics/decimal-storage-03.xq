(: Arithmetic and rounding on xs:decimal values of every size. :)
let $a := xs:decimal( "123.25" )
let $b := xs:decimal( "123456789012345678901234567890.5" )
return string-join(
  for $x in ( $a + $b, $b - $a, $a * $b, $a div 4, $b div 2, $b mod $a, -$a,
              round( $a ), round( -2.5 ), round( 2.5 ),
              round-half-to-even( 2.5 ), round-half-to-even( 3.5 ),
              round-half-to-even( $a, 1 ), floor( -$a ), ceiling( $a ),
              9223372036854775807 + 0.5,
              xs:decimal( "-9223372036854775808" ) - 0.5,
              xs:decimal( "18446744073709551615" ) * 1.5,
              sum( ( 0.1, 0.2, 0.3 ) ) eq 0.6 )
  return string( $x ),
  " " )
