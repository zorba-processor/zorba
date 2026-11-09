import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

m:create(xs:QName("m:r"),
         (<e id="1" t="d" v="0.1"/>, <e id="2" t="f" v="0.1"/>,
          <e id="3" t="dec" v="0.1"/>),
         xs:QName("m:RRange"));

(: Bounds in decreasing order as xs:decimal values that are equal once
   promoted to the type of the keys. :)
fn:string-join((
  m:range-value("bounds equal as xs:double", xs:QName("m:r"),
                xs:QName("m:RRange"), 0.1000000000000000001, 0.1),
  m:range-value("bounds equal as xs:float", xs:QName("m:r"),
                xs:QName("m:RRange"), 0.100000003, 0.1),
  m:range-value("bounds in order", xs:QName("m:r"), xs:QName("m:RRange"),
                0.1, 0.1000000000000000001)),
  "&#10;")
