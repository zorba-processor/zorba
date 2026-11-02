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

(: XQuery 3.0 3.10.8: "All the non-empty orderspec values must be convertible to a
   common type by subtype substitution and/or type promotion. The ordering is
   performed in the least common type that has a gt operator."
   Each case returns the input positions in result order. Values equal in the
   common type keep their input order (stable). :)
declare function local:r($p as xs:integer*) as xs:string { string-join($p ! string(.), ",") };
(
  (: static type xs:decimal, common type xs:decimal :)
  try { local:r(for $x at $p in (1.5, 1, 1.2) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (1, 1.5, 1.2) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775808.0, 9223372036854775807) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775807, 9223372036854775808.0) stable order by $x descending return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775807.5, 9223372036854775807, 9223372036854775808.0) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775807, 9223372036854775807.5) stable order by $x descending return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (18446744073709551616.0, 9223372036854775807, -9223372036854775809.0, xs:integer("-9223372036854775808"), 1) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (18446744073709551616.0, 9223372036854775807, -9223372036854775809.0, xs:integer("-9223372036854775808"), 1) stable order by $x descending return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775807.0, 9223372036854775807) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775807, 9223372036854775807.0) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775808.0, xs:long("9223372036854775807")) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775807.5, xs:long("9223372036854775807"), 9223372036854775808.0) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (xs:long("-9223372036854775808"), -9223372036854775809.0) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  (: static type xs:integer :)
  try { local:r(for $x at $p in (xs:unsignedLong("9223372036854775807"), -1, xs:nonNegativeInteger(\"5\"), xs:long("9223372036854775807"), 9223372036854775807) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  (: common type xs:double :)
  try { local:r(for $x at $p in (9.223372036854775808E18, 9223372036854775807, 1) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9.223372036854774784E18, xs:long("9223372036854775807"), -9.223372036854775808E18) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (1.8446744073709551616E19, 18446744073709551616.0, 9223372036854775808.0) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  (: all three equal as xs:double, although 9223372036854775807 lt 9223372036854775808.0 as xs:decimal :)
  try { local:r(for $x at $p in (9223372036854775808.0, 9223372036854775807, 9.223372036854775808E18) stable order by $x return $p) } catch * { local-name-from-QName($err:code) },
  try { local:r(for $x at $p in (9223372036854775807.5, xs:long("9223372036854775807"), 9.223372036854775808E18, 9223372036854775807) stable order by $x return $p) } catch * { local-name-from-QName($err:code) }
)
