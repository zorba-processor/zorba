(: Values that are equal under value comparison (after numeric promotion)
   must be recognized as duplicates by fn:distinct-values. :)
(
  (: integer promoted to double: 2^53+1 rounds to 2^53 :)
  count(distinct-values((9007199254740993, 9.007199254740992E15))),
  count(distinct-values((9.007199254740992E15, 9007199254740993))),
  (: 2^53+3 rounds to 2^53+4 :)
  count(distinct-values((9007199254740995, 9.007199254740996E15))),
  count(distinct-values((xs:decimal("9007199254740993"), 9.007199254740992E15))),
  (: 2^63-1 rounds to 2^63 :)
  count(distinct-values((9223372036854775807, 9.223372036854775807E18))),
  (: 2^64-1 rounds to 2^64 :)
  count(distinct-values((xs:decimal("18446744073709551615"), 1.8446744073709552E19))),
  count(distinct-values((xs:decimal("-9223372036854775809"), -9.223372036854775808E18))),
  (: two integers are compared exactly :)
  count(distinct-values((9007199254740993, 9007199254740992))),
  (: fractional values and mixed fractional types :)
  count(distinct-values((xs:decimal("0.1"), 0.1e0, xs:double("0.1")))),
  count(distinct-values((xs:float("1.5"), 1.5e0, xs:decimal("1.5")))),
  count(distinct-values((xs:decimal("123456789.5"), 123456789.5e0))),
  (: negative zero equals zero :)
  count(distinct-values((0, -0e0, xs:float("-0"), xs:decimal("0.0")))),
  (: NaN is equal to NaN, of any floating-point type :)
  count(distinct-values((xs:double("NaN"), xs:float("NaN"), 1)))
)
