import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:em"),
         (<e id="1" v="b"/>, <e id="2"/>, <e id="3" v="a"/>, <e id="4"/>),
         (xs:QName("m:EmRange"), xs:QName("m:EmTypedRange")));

(: A range probe without bounds selects every domain node, also those whose
   key is the empty sequence. :)
fn:string-join((
  for $index in (xs:QName("m:EmRange"), xs:QName("m:EmTypedRange"))
  let $name := fn:local-name-from-QName($index)
  return (
    m:range(fn:concat($name, " general, no bounds"), xs:QName("m:em"),
            $index, (), (), fn:true()),
    m:result(fn:concat($name, " value, no bounds"),
             m:ids(idml:probe-index-range-value($index, (), (),
                                                fn:false(), fn:false(),
                                                fn:false(), fn:false())),
             m:ids(m:elements(xs:QName("m:em")))))),
  "&#10;")
