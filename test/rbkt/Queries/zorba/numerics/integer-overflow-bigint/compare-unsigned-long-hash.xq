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

declare namespace err = "http://www.w3.org/2005/xqt-errors";

(: distinct-values, group by, deep-equal, order by and index-of over
   xs:unsignedLong above 2^63-1. :)
for $s in (
  [ xs:unsignedLong("18446744073709551615"), 1.8446744073709551616E19 ], [ xs:unsignedLong("18446744073709551615"), 18446744073709551615.0 ], [ xs:unsignedLong("18446744073709551615"), 18446744073709551616.0 ], [ xs:unsignedLong("18446744073709551615"), 9223372036854775807 ], [ 9223372036854775807, xs:unsignedLong("18446744073709551615") ], [ xs:unsignedLong("18446744073709551615"), xs:long("9223372036854775807") ]
)
let $v := ($s(1), $s(2))
return concat(
  count(distinct-values($v)),
  count(for $x in $v group by $k := $x return 1),
  deep-equal($s(1), $s(2)))
,
string-join(for $x at $p in (xs:unsignedLong("18446744073709551615"), 9223372036854775807, -1, xs:long("9223372036854775807")) stable order by $x return string($p), ","),
string-join(for $x at $p in (1.8446744073709551616E19, xs:unsignedLong("18446744073709551615"), 9223372036854775808.0) stable order by $x return string($p), ","),
string-join(index-of((xs:unsignedLong("18446744073709551615"), 9223372036854775807), 9223372036854775807) ! string(.), ","),
string-join(index-of((9223372036854775807, xs:unsignedLong("18446744073709551615")), xs:unsignedLong("18446744073709551615")) ! string(.), ","),
string-join(index-of((xs:unsignedLong("18446744073709551615"), 18446744073709551616.0), 18446744073709551616.0) ! string(.), ",")
