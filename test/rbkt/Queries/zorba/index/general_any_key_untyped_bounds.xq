import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:a"),
         (<e id="1" v="a"/>, <e id="2" v="b"/>, <e id="3" v="c"/>,
          <e id="4" v="d"/>, <e id="5" v="e"/>),
         xs:QName("m:ARange"));

(: In a value range probe of an index whose key type is xs:anyAtomicType*,
   untyped bounds compare as strings, as in an index without a key type. :)
let $lower := xs:untypedAtomic("b")
let $upper := xs:untypedAtomic("d")
return fn:string-join((
  fn:concat("indexed: ",
            m:ids(idml:probe-index-range-value(xs:QName("m:ARange"),
                                               $lower, $upper,
                                               fn:true(), fn:true(),
                                               fn:true(), fn:true()))),
  fn:concat("unindexed: ",
            m:ids(for $e in m:elements(xs:QName("m:a"))
                  where $lower le fn:data($e/@v) and fn:data($e/@v) le $upper
                  return $e))),
  "&#10;")
