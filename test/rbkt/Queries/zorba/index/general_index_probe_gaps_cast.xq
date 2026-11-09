import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

m:create(xs:QName("m:y"),
         (<e id="1" t="y" v="2001"/>, <e id="2" t="y" v="1999"/>,
          <e id="3" t="y" v="1998Z"/>),
         xs:QName("m:YEquality"));
m:create(xs:QName("m:h"),
         (<e id="1" v="12345678901234567890"/>, <e id="2" v="1E10"/>,
          <e id="3" v="1234"/>),
         (xs:QName("m:HEquality"), xs:QName("m:HRange")));
m:create(xs:QName("m:d"), <e id="1" v="P10Y"/>, xs:QName("m:DEquality"));
m:create(xs:QName("m:t"),
         (<e id="1" t="x" v="1E10"/>, <e id="2" t="b" v="1E10"/>),
         xs:QName("m:TEquality"));

(: An untyped value is cast to the type of the key it is compared with. :)
fn:string-join((
  m:point("gYear keys, untyped integer", xs:QName("m:y"),
          xs:QName("m:YEquality"), xs:untypedAtomic("2001")),
  m:point("gYear keys, untyped with timezone", xs:QName("m:y"),
          xs:QName("m:YEquality"), xs:untypedAtomic("1998Z")),
  for $probe in (
    ["hexBinary decimal", xs:hexBinary("12345678901234567890")],
    ["base64Binary decimal", xs:base64Binary("12345678901234567890")],
    ["hexBinary double", xs:hexBinary("1E10")],
    ["base64Binary double", xs:base64Binary("1E10")],
    ["hexBinary integer", xs:hexBinary("1234")],
    ["base64Binary integer", xs:base64Binary("1234")])
  return m:point(fn:concat("untyped keys, ", $probe(1)), xs:QName("m:h"),
                 xs:QName("m:HEquality"), $probe(2)),
  m:point-error("range index, base64Binary", xs:QName("m:HRange"),
                xs:base64Binary("1234")),
  m:point("untyped duration key, base64Binary", xs:QName("m:d"),
          xs:QName("m:DEquality"), xs:base64Binary("P10Y")),
  m:point("hexBinary and base64Binary keys, untyped", xs:QName("m:t"),
          xs:QName("m:TEquality"), xs:untypedAtomic("1E10"))),
  "&#10;")
