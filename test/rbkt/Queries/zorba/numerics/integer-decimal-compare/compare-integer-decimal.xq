(: XPath 3.0 / XQuery 3.0 B.2: "If an operator in the operator mapping tables
   expects an operand of type ET, that operator can be applied to an operand of
   type AT if type AT can be converted to type ET by a combination of type
   promotion and subtype substitution."
   B.1: "A value of type xs:decimal (or any type derived by restriction from
   xs:decimal) can be promoted to either of the types xs:float or xs:double. The
   result of this promotion is created by casting the original value to the
   required type. This kind of promotion may cause loss of precision."
   B.1: "A function that expects a parameter $p of type xs:decimal can be invoked
   with a value of type xs:integer. This is an example of subtype substitution."
   F&O 3.0 4.2: "If the two operands are not of the same type, subtype
   substitution and numeric type promotion are used to obtain two operands of the
   same type."
   F&O 3.0 op:numeric-equal: "Returns true if and only if the value of $arg1 is
   equal to the value of $arg2."
   So xs:integer against xs:decimal compares exactly, as xs:decimal; any
   xs:integer or xs:decimal against xs:double compares as xs:double, after
   rounding. :)

(: xs:integer and xs:decimal around -2^63, 2^63 and 2^64, static type xs:decimal.
   One row per left operand, one letter per right operand: l(ess), e(qual),
   g(reater), x if eq, ne, lt, le, gt, ge, =, <, > disagree. An error is reported
   in place of its row. :)
let $v := (-9223372036854775809.0, xs:integer("-9223372036854775808"), -9223372036854775808.0, 1, 1.5, 9223372036854775807, 9223372036854775807.0, 9223372036854775807.5, 9223372036854775808.0, 18446744073709551616.0)
for $a in $v
return
  try {
    string-join(
      for $b in $v
      return
        if ($a eq $b and $a = $b and $a le $b and $a ge $b and
            not($a ne $b) and not($a lt $b) and not($a gt $b)) then "e"
        else if ($a lt $b and $a < $b and $a le $b and $a ne $b and
                 not($a eq $b) and not($a gt $b) and not($a ge $b)) then "l"
        else if ($a gt $b and $a > $b and $a ge $b and $a ne $b and
                 not($a eq $b) and not($a lt $b) and not($a le $b)) then "g"
        else "x",
      "")
  } catch * { local-name-from-QName($err:code) }
