import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:u"),
         (<e id="1" v="a"/>, <e id="2" v="b"/>, <e id="3" v="c"/>),
         (xs:QName("m:UEquality"), xs:QName("m:URange")));

(: In the value probes of an index whose key type is xs:untypedAtomic*, an
   untyped search key or bound compares as a string, as in a value comparison
   with an untyped value. :)
let $b := xs:untypedAtomic("b")
return fn:string-join((
  fn:concat("point: ",
            m:ids(idml:probe-index-point-value(xs:QName("m:UEquality"), $b))),
  fn:concat("range: ",
            m:ids(idml:probe-index-range-value(xs:QName("m:URange"),
                                               xs:untypedAtomic("a"), $b,
                                               fn:true(), fn:true(),
                                               fn:true(), fn:true()))),
  fn:concat("unindexed: ",
            m:ids(for $e in m:elements(xs:QName("m:u"))
                  where fn:data($e/@v) eq $b
                  return $e))),
  "&#10;")
