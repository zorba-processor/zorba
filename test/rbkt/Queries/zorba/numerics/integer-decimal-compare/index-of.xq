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

(: F&O 3.0 fn:index-of: positions of "items that are equal to $search",
   compared with eq. Static types xs:decimal and xs:anyAtomicType. :)
declare function local:r($p as xs:integer*) as xs:string { concat("[", string-join($p ! string(.), ","), "]") };
(
  try { local:r(index-of((1, 1.5), 1.5)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((1.5, 1), 1)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((9223372036854775807, 9223372036854775808.0), 9223372036854775808.0)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((9223372036854775808.0, 9223372036854775807), 9223372036854775807)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((9223372036854775807, 9223372036854775807.0), 9223372036854775807.0)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((9223372036854775807, 9223372036854775807.5), 9223372036854775807.5)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((xs:integer("-9223372036854775808"), -9223372036854775809.0), -9223372036854775809.0)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((xs:long("9223372036854775807"), 9223372036854775808.0), 9223372036854775808.0)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((xs:long("9223372036854775807"), 9223372036854775807.0), 9223372036854775807.0)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((9223372036854775807, 9223372036854775808.0), 9.223372036854775808E18)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((9.223372036854775808E18, 9223372036854775807), 9223372036854775807)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((9223372036854775807, xs:long("9223372036854775807")), xs:long("9223372036854775807"))) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((xs:unsignedLong("18446744073709551615"), 9223372036854775807), 9223372036854775807)) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((9223372036854775807, xs:unsignedLong("18446744073709551615")), xs:unsignedLong("18446744073709551615"))) } catch * { local-name-from-QName($err:code) },
  try { local:r(index-of((xs:unsignedLong("18446744073709551615"), 18446744073709551616.0), 18446744073709551616.0)) } catch * { local-name-from-QName($err:code) }
)
