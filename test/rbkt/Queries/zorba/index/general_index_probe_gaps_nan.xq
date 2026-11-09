import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

m:create(xs:QName("m:n"),
         (<e id="1" t="d" v="NaN"/>, <e id="2" v="NaN"/>,
          <e id="3" t="d" v="1"/>, <e id="4" t="d" v="NaN"/>,
          <e id="5" v="2"/>),
         xs:QName("m:NRange"));
m:create(xs:QName("m:nt"),
         (<e id="1" t="d" v="NaN"/>, <e id="2" t="d" v="1"/>,
          <e id="3" t="d" v="NaN"/>, <e id="4" t="d" v="2"/>),
         xs:QName("m:NTypedRange"));

(: NaN is equal to, less than and greater than no number; a probe without
   bounds selects every node. :)
fn:string-join((
  m:point("NRange 1", xs:QName("m:n"), xs:QName("m:NRange"), 1e0),
  m:point("NRange NaN", xs:QName("m:n"), xs:QName("m:NRange"),
          xs:double("NaN")),
  m:point("NRange untyped NaN", xs:QName("m:n"), xs:QName("m:NRange"),
          xs:untypedAtomic("NaN")),
  m:range("NRange lower 0", xs:QName("m:n"), xs:QName("m:NRange"),
          0e0, (), true()),
  m:range("NRange upper 5", xs:QName("m:n"), xs:QName("m:NRange"),
          (), 5e0, true()),
  m:range("NRange no bounds", xs:QName("m:n"), xs:QName("m:NRange"),
          (), (), true()),
  m:range("NRange exclusive lower 1", xs:QName("m:n"), xs:QName("m:NRange"),
          1e0, (), false()),
  m:range("NRange exclusive upper 2", xs:QName("m:n"), xs:QName("m:NRange"),
          (), 2e0, false()),
  m:point("NTypedRange 2", xs:QName("m:nt"), xs:QName("m:NTypedRange"), 2e0),
  m:point("NTypedRange NaN", xs:QName("m:nt"), xs:QName("m:NTypedRange"),
          xs:double("NaN")),
  m:range("NTypedRange lower NaN", xs:QName("m:nt"),
          xs:QName("m:NTypedRange"), xs:double("NaN"), (), true()),
  m:range("NTypedRange upper 5", xs:QName("m:nt"),
          xs:QName("m:NTypedRange"), (), 5e0, true()),
  m:range("NTypedRange exclusive lower 1", xs:QName("m:nt"),
          xs:QName("m:NTypedRange"), 1e0, (), false()),
  m:range("NTypedRange exclusive upper 2", xs:QName("m:nt"),
          xs:QName("m:NTypedRange"), (), 2e0, false()),
  m:range("NTypedRange no bounds", xs:QName("m:nt"),
          xs:QName("m:NTypedRange"), (), (), true())),
  "&#10;")
