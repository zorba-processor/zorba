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

(: F&O 3.0 fn:distinct-values: "Values are compared using the eq operator". When
   "A eq B, B eq C, but A ne C" the number of items is implementation dependent
   "subject only to the constraints that (a) no two items in the result sequence
   compare equal to each other, and (b) every input item that does not appear in
   the result sequence compares equal to some item that does appear in the result
   sequence." tN checks that on three values. Equal values must hash equally, or
   the set does not find the duplicate. :)
declare function local:spec($r as xs:anyAtomicType*, $input as xs:anyAtomicType*) as xs:boolean
{
  (every $i in 1 to count($r), $j in 1 to count($r)
   satisfies $i eq $j or $r[$i] ne $r[$j]) and
  (every $x in $input satisfies some $y in $r satisfies $x eq $y)
};

(
  try { concat("i-d:", count(distinct-values((9223372036854775807, 9223372036854775808.0)))) } catch * { concat("i-d:", local-name-from-QName($err:code)) },
  try { concat("d-i:", count(distinct-values((9223372036854775808.0, 9223372036854775807)))) } catch * { concat("d-i:", local-name-from-QName($err:code)) },
  try { concat("i-di:", count(distinct-values((9223372036854775807, 9223372036854775807.0)))) } catch * { concat("i-di:", local-name-from-QName($err:code)) },
  try { concat("di-i:", count(distinct-values((9223372036854775807.0, 9223372036854775807)))) } catch * { concat("di-i:", local-name-from-QName($err:code)) },
  try { concat("i-d2:", count(distinct-values((9223372036854775807, 9223372036854775807.5)))) } catch * { concat("i-d2:", local-name-from-QName($err:code)) },
  try { concat("d2-i:", count(distinct-values((9223372036854775807.5, 9223372036854775807)))) } catch * { concat("d2-i:", local-name-from-QName($err:code)) },
  try { concat("imin-dme:", count(distinct-values((xs:integer("-9223372036854775808"), -9223372036854775808.0)))) } catch * { concat("imin-dme:", local-name-from-QName($err:code)) },
  try { concat("dme-imin:", count(distinct-values((-9223372036854775808.0, xs:integer("-9223372036854775808"))))) } catch * { concat("dme-imin:", local-name-from-QName($err:code)) },
  try { concat("imin-dm:", count(distinct-values((xs:integer("-9223372036854775808"), -9223372036854775809.0)))) } catch * { concat("imin-dm:", local-name-from-QName($err:code)) },
  try { concat("dm-imin:", count(distinct-values((-9223372036854775809.0, xs:integer("-9223372036854775808"))))) } catch * { concat("dm-imin:", local-name-from-QName($err:code)) },
  try { concat("i-d64:", count(distinct-values((9223372036854775807, 18446744073709551616.0)))) } catch * { concat("i-d64:", local-name-from-QName($err:code)) },
  try { concat("d64-i:", count(distinct-values((18446744073709551616.0, 9223372036854775807)))) } catch * { concat("d64-i:", local-name-from-QName($err:code)) },
  try { concat("one-1.5:", count(distinct-values((1, 1.5)))) } catch * { concat("one-1.5:", local-name-from-QName($err:code)) },
  try { concat("1.5-one:", count(distinct-values((1.5, 1)))) } catch * { concat("1.5-one:", local-name-from-QName($err:code)) },
  try { concat("l-d:", count(distinct-values((xs:long("9223372036854775807"), 9223372036854775808.0)))) } catch * { concat("l-d:", local-name-from-QName($err:code)) },
  try { concat("d-l:", count(distinct-values((9223372036854775808.0, xs:long("9223372036854775807"))))) } catch * { concat("d-l:", local-name-from-QName($err:code)) },
  try { concat("l-di:", count(distinct-values((xs:long("9223372036854775807"), 9223372036854775807.0)))) } catch * { concat("l-di:", local-name-from-QName($err:code)) },
  try { concat("di-l:", count(distinct-values((9223372036854775807.0, xs:long("9223372036854775807"))))) } catch * { concat("di-l:", local-name-from-QName($err:code)) },
  try { concat("l-d2:", count(distinct-values((xs:long("9223372036854775807"), 9223372036854775807.5)))) } catch * { concat("l-d2:", local-name-from-QName($err:code)) },
  try { concat("d2-l:", count(distinct-values((9223372036854775807.5, xs:long("9223372036854775807"))))) } catch * { concat("d2-l:", local-name-from-QName($err:code)) },
  try { concat("lmin-dme:", count(distinct-values((xs:long("-9223372036854775808"), -9223372036854775808.0)))) } catch * { concat("lmin-dme:", local-name-from-QName($err:code)) },
  try { concat("lmin-dm:", count(distinct-values((xs:long("-9223372036854775808"), -9223372036854775809.0)))) } catch * { concat("lmin-dm:", local-name-from-QName($err:code)) },
  try { concat("l-i:", count(distinct-values((xs:long("9223372036854775807"), 9223372036854775807)))) } catch * { concat("l-i:", local-name-from-QName($err:code)) },
  try { concat("i-l:", count(distinct-values((9223372036854775807, xs:long("9223372036854775807"))))) } catch * { concat("i-l:", local-name-from-QName($err:code)) },
  try { concat("lmin-imin:", count(distinct-values((xs:long("-9223372036854775808"), xs:integer("-9223372036854775808"))))) } catch * { concat("lmin-imin:", local-name-from-QName($err:code)) },
  try { concat("i53-d53h:", count(distinct-values((9007199254740993, 9007199254740993.5)))) } catch * { concat("i53-d53h:", local-name-from-QName($err:code)) },
  try { concat("d53h-i53:", count(distinct-values((9007199254740993.5, 9007199254740993)))) } catch * { concat("d53h-i53:", local-name-from-QName($err:code)) },
  try { concat("l53-i53:", count(distinct-values((xs:long("9007199254740993"), 9007199254740993)))) } catch * { concat("l53-i53:", local-name-from-QName($err:code)) },
  try { concat("i53-l53:", count(distinct-values((9007199254740993, xs:long("9007199254740993"))))) } catch * { concat("i53-l53:", local-name-from-QName($err:code)) },
  try { concat("l53-d53:", count(distinct-values((xs:long("9007199254740993"), 9007199254740993.0)))) } catch * { concat("l53-d53:", local-name-from-QName($err:code)) },
  try { concat("l53-d53h:", count(distinct-values((xs:long("9007199254740993"), 9007199254740993.5)))) } catch * { concat("l53-d53h:", local-name-from-QName($err:code)) },
  try { concat("l53-x53:", count(distinct-values((xs:long("9007199254740993"), 9.007199254740992E15)))) } catch * { concat("l53-x53:", local-name-from-QName($err:code)) },
  try { concat("i53-x53:", count(distinct-values((9007199254740993, 9.007199254740992E15)))) } catch * { concat("i53-x53:", local-name-from-QName($err:code)) },
  try { concat("i-x:", count(distinct-values((9223372036854775807, 9.223372036854775808E18)))) } catch * { concat("i-x:", local-name-from-QName($err:code)) },
  try { concat("x-i:", count(distinct-values((9.223372036854775808E18, 9223372036854775807)))) } catch * { concat("x-i:", local-name-from-QName($err:code)) },
  try { concat("l-x:", count(distinct-values((xs:long("9223372036854775807"), 9.223372036854775808E18)))) } catch * { concat("l-x:", local-name-from-QName($err:code)) },
  try { concat("x-l:", count(distinct-values((9.223372036854775808E18, xs:long("9223372036854775807"))))) } catch * { concat("x-l:", local-name-from-QName($err:code)) },
  try { concat("d-x:", count(distinct-values((9223372036854775808.0, 9.223372036854775808E18)))) } catch * { concat("d-x:", local-name-from-QName($err:code)) },
  try { concat("x-d:", count(distinct-values((9.223372036854775808E18, 9223372036854775808.0)))) } catch * { concat("x-d:", local-name-from-QName($err:code)) },
  try { concat("d2-x:", count(distinct-values((9223372036854775807.5, 9.223372036854775808E18)))) } catch * { concat("d2-x:", local-name-from-QName($err:code)) },
  try { concat("imin-nx:", count(distinct-values((xs:integer("-9223372036854775808"), -9.223372036854775808E18)))) } catch * { concat("imin-nx:", local-name-from-QName($err:code)) },
  try { concat("lmin-nx:", count(distinct-values((xs:long("-9223372036854775808"), -9.223372036854775808E18)))) } catch * { concat("lmin-nx:", local-name-from-QName($err:code)) },
  try { concat("i-x0:", count(distinct-values((9223372036854775807, 9.223372036854774784E18)))) } catch * { concat("i-x0:", local-name-from-QName($err:code)) },
  try { concat("l-x0:", count(distinct-values((xs:long("9223372036854775807"), 9.223372036854774784E18)))) } catch * { concat("l-x0:", local-name-from-QName($err:code)) },
  try { concat("d64-x64:", count(distinct-values((18446744073709551616.0, 1.8446744073709551616E19)))) } catch * { concat("d64-x64:", local-name-from-QName($err:code)) },
  try { concat("u-x64:", count(distinct-values((xs:unsignedLong("18446744073709551615"), 1.8446744073709551616E19)))) } catch * { concat("u-x64:", local-name-from-QName($err:code)) },
  try { concat("u-du:", count(distinct-values((xs:unsignedLong("18446744073709551615"), 18446744073709551615.0)))) } catch * { concat("u-du:", local-name-from-QName($err:code)) },
  try { concat("u-d64:", count(distinct-values((xs:unsignedLong("18446744073709551615"), 18446744073709551616.0)))) } catch * { concat("u-d64:", local-name-from-QName($err:code)) },
  try { concat("u-i:", count(distinct-values((xs:unsignedLong("18446744073709551615"), 9223372036854775807)))) } catch * { concat("u-i:", local-name-from-QName($err:code)) },
  try { concat("i-u:", count(distinct-values((9223372036854775807, xs:unsignedLong("18446744073709551615"))))) } catch * { concat("i-u:", local-name-from-QName($err:code)) },
  try { concat("u-l:", count(distinct-values((xs:unsignedLong("18446744073709551615"), xs:long("9223372036854775807"))))) } catch * { concat("u-l:", local-name-from-QName($err:code)) },
  try { concat("u63-i:", count(distinct-values((xs:unsignedLong("9223372036854775807"), 9223372036854775807)))) } catch * { concat("u63-i:", local-name-from-QName($err:code)) },
  try { concat("i-u63:", count(distinct-values((9223372036854775807, xs:unsignedLong("9223372036854775807"))))) } catch * { concat("i-u63:", local-name-from-QName($err:code)) },
  try { concat("u63-l:", count(distinct-values((xs:unsignedLong("9223372036854775807"), xs:long("9223372036854775807"))))) } catch * { concat("u63-l:", local-name-from-QName($err:code)) },
  try { concat("u63-di:", count(distinct-values((xs:unsignedLong("9223372036854775807"), 9223372036854775807.0)))) } catch * { concat("u63-di:", local-name-from-QName($err:code)) },
  try { concat("u63-x:", count(distinct-values((xs:unsignedLong("9223372036854775807"), 9.223372036854775808E18)))) } catch * { concat("u63-x:", local-name-from-QName($err:code)) },
  try { concat("t1:", local:spec(distinct-values((9223372036854775807, 9223372036854775808.0, 9.223372036854775808E18)), (9223372036854775807, 9223372036854775808.0, 9.223372036854775808E18))) } catch * { concat("t1:", local-name-from-QName($err:code)) },
  try { concat("t2:", local:spec(distinct-values((9.223372036854775808E18, 9223372036854775807, 9223372036854775808.0)), (9.223372036854775808E18, 9223372036854775807, 9223372036854775808.0))) } catch * { concat("t2:", local-name-from-QName($err:code)) },
  try { concat("t3:", local:spec(distinct-values((9223372036854775808.0, 9223372036854775807, 9.223372036854775808E18)), (9223372036854775808.0, 9223372036854775807, 9.223372036854775808E18))) } catch * { concat("t3:", local-name-from-QName($err:code)) },
  try { concat("t4:", local:spec(distinct-values((9223372036854775807, 9223372036854775807.5, 9.223372036854775808E18)), (9223372036854775807, 9223372036854775807.5, 9.223372036854775808E18))) } catch * { concat("t4:", local-name-from-QName($err:code)) },
  try { concat("t5:", local:spec(distinct-values((xs:long("9223372036854775807"), 9223372036854775808.0, 9.223372036854775808E18)), (xs:long("9223372036854775807"), 9223372036854775808.0, 9.223372036854775808E18))) } catch * { concat("t5:", local-name-from-QName($err:code)) },
  try { concat("t6:", local:spec(distinct-values((xs:unsignedLong("18446744073709551615"), 18446744073709551616.0, 1.8446744073709551616E19)), (xs:unsignedLong("18446744073709551615"), 18446744073709551616.0, 1.8446744073709551616E19))) } catch * { concat("t6:", local-name-from-QName($err:code)) }
)
