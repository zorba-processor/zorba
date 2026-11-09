import module namespace m = "http://www.example.com/general-numeric-promotion" at
                             "general_numeric_promotion.xqlib";

m:init-limits();

fn:string-join((
  for $index in (xs:QName("m:VRange"), xs:QName("m:VEquality"))
  let $n := fn:local-name-from-QName($index)
  for $probe in (
    ["double 2^63", xs:double("9.223372036854775808E18")],
    ["float 2^63", xs:float("9.223372E18")],
    ["untyped xs:long max", xs:untypedAtomic("9223372036854775807")],
    ["double -2^63", xs:double("-9.223372036854775808E18")],
    ["xs:unsignedLong max", xs:unsignedLong("18446744073709551615")],
    ["double 2^64", xs:double("1.8446744073709552E19")],
    ["decimal 2^63", xs:decimal("9223372036854775808")],
    ["double 2^53", 9.007199254740992E15],
    ["double 2^53+2", 9.007199254740994E15],
    ["double -0", -0e0],
    ["float -0", xs:float("-0")],
    ["double INF", xs:double("INF")],
    ["double -INF", xs:double("-INF")],
    ["float INF", xs:float("INF")])
  return m:point(fn:concat($n, " ", $probe(1)), xs:QName("m:v"), $index,
                 $probe(2)),
  for $index in (xs:QName("m:VRange"), xs:QName("m:VEquality"))
  let $n := fn:local-name-from-QName($index)
  for $probe in (
    ["double 2^63", xs:double("9.223372036854775808E18")],
    ["float 2^63", xs:float("9.223372E18")],
    ["double 2^53+2", 9.007199254740994E15],
    ["float -0", xs:float("-0")],
    ["float INF", xs:float("INF")])
  return m:point-value(fn:concat($n, " value ", $probe(1)), $index, $probe(2)),
  m:range("range double 2^53 lower", xs:QName("m:v"), xs:QName("m:VRange"),
          9.007199254740992E15, (), true()),
  m:range("range exclusive float -0 upper", xs:QName("m:v"),
          xs:QName("m:VRange"), (), xs:float("-0"), false()),
  m:range("range float INF lower", xs:QName("m:v"), xs:QName("m:VRange"),
          xs:float("INF"), (), true()),
  m:range("range double 2^63 lower", xs:QName("m:v"), xs:QName("m:VRange"),
          xs:double("9.223372036854775808E18"), (), true()),
  m:range-value("range value float 2^63 lower", xs:float("9.223372E18"), ()),
  m:range-value("range value double 2^53 both", 9.007199254740992E15,
                9.007199254740994E15),
  m:range-value("range value float bounds", xs:float("-1"), xs:float("0.5"))),
  "&#10;")
