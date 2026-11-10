import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

m:create(xs:QName("m:bo"),
         (<e id="1" v="true"/>, <e id="2" v="1"/>, <e id="3" v="0"/>,
          <e id="4" v="false"/>, <e id="5" t="bool" v="true"/>,
          <e id="6" t="bool" v="false"/>),
         (xs:QName("m:BoEquality"), xs:QName("m:BoRange")));

(: Compared with an xs:boolean, an untyped value is cast to xs:boolean. :)
fn:string-join((
  m:point("BoEquality, true", xs:QName("m:bo"), xs:QName("m:BoEquality"),
          fn:true()),
  m:point("BoEquality, false", xs:QName("m:bo"), xs:QName("m:BoEquality"),
          fn:false()),
  m:point("BoEquality, untyped 1", xs:QName("m:bo"),
          xs:QName("m:BoEquality"), xs:untypedAtomic("1")),
  m:point("BoEquality, untyped false", xs:QName("m:bo"),
          xs:QName("m:BoEquality"), xs:untypedAtomic("false")),
  m:point("BoRange, true", xs:QName("m:bo"), xs:QName("m:BoRange"),
          fn:true()),
  m:point("BoRange, untyped 1", xs:QName("m:bo"), xs:QName("m:BoRange"),
          xs:untypedAtomic("1")),
  m:range("BoRange, above false", xs:QName("m:bo"), xs:QName("m:BoRange"),
          fn:false(), (), fn:false()),
  m:range("BoRange, up to untyped 0", xs:QName("m:bo"),
          xs:QName("m:BoRange"), (), xs:untypedAtomic("0"), fn:true())),
  "&#10;")
