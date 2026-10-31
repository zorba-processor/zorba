(: F&O 3.0 4.7.5: the rounded number is the adjusted number converted to
   xs:decimal; xs:decimal and xs:integer values must not lose digits by
   going through xs:double, and an xs:untypedAtomic value is cast to
   xs:double by the function conversion rules. :)
string-join((
  format-number(xs:decimal('79228162514264337593543950335'), '0%'),
  format-number(xs:decimal('1.10203040506070809'), '0.00000000000000000%'),
  format-number(123456789012345678.123456789012345678, '#,##0.000000000000000000'),
  format-number(9223372036854775807, '#'),
  format-number(-9223372036854775807, '#,###'),
  format-number(0.125, '0.00'),
  format-number(0.135, '0.00'),
  format-number(1E25, '#,######'),
  format-number(1E25, '#####################'),
  format-number(xs:float('0.1'), '0.0#########'),
  format-number(1e308, '0%'),
  format-number(xs:untypedAtomic('1'), '#')
), '|')
