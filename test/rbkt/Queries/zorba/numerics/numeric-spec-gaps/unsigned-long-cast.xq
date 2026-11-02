(: XSD 1.1 Part 2 3.4.21 unsignedLong: "maxInclusive to be 18446744073709551615";
   F&O 3.0 19.1: a value in the range of the target casts; 19.2 casting to a type
   derived from xs:integer goes through xs:integer's value, truncating a fraction,
   and raises FORG0001 outside the target's range. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare function local:c($v as xs:anyAtomicType) as xs:string
{
  try { string(xs:unsignedLong($v)) } catch * { local-name-from-QName($err:code) }
};
(
  local:c("9223372036854775807"),
  local:c("9223372036854775808"),
  local:c("18446744073709551615"),
  local:c(" 18446744073709551615 "),
  local:c("+18446744073709551615"),
  local:c("000018446744073709551615"),
  local:c(xs:untypedAtomic("18446744073709551615")),
  local:c(9223372036854775808.0),
  local:c(18446744073709551615.0),
  local:c(18446744073709551615.9),
  local:c(9.223372036854775808E18),
  local:c(1.8446744073709549568E19),
  local:c(xs:float("9223372036854775808")),
  local:c(xs:float("1E19")),
  local:c(xs:unsignedLong("18446744073709551615")),
  (: outside the range: FORG0001 :)
  local:c("18446744073709551616"),
  local:c("-1"),
  local:c(18446744073709551616.0),
  local:c(1.8446744073709551616E19),
  local:c(xs:float("1.8446744073709551616E19")),
  (: not a number: FORG0001; NaN and INF: FOCA0002 :)
  local:c("1.5"),
  local:c("abc"),
  local:c(xs:double("NaN")),
  local:c(xs:double("INF")),
  local:c(xs:float("-INF")),
  (: fraction truncated, as for the smaller values :)
  local:c(0.9),
  local:c(-0.9),
  local:c(1.5e0),
  "18446744073709551615" castable as xs:unsignedLong,
  "18446744073709551616" castable as xs:unsignedLong,
  18446744073709551615.0 castable as xs:unsignedLong,
  18446744073709551616.0 castable as xs:unsignedLong,
  xs:unsignedLong("18446744073709551615") instance of xs:unsignedLong,
  xs:unsignedLong("18446744073709551615") instance of xs:nonNegativeInteger,
  xs:unsignedLong("18446744073709551615") instance of xs:integer,
  xs:unsignedLong("18446744073709551615") instance of xs:long
)
