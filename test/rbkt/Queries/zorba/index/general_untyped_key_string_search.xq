import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:u"),
         (<e id="1" v="a"/>, <e id="2" v="b"/>, <e id="3" v="c"/>),
         (xs:QName("m:UEquality"), xs:QName("m:URange")));

(: An index whose key type is xs:untypedAtomic* is probed with xs:string and
   xs:anyURI search keys, which a comparison with an untyped value accepts. :)
fn:string-join((
  fn:concat("point string: ",
            m:ids(idml:probe-index-point-value(xs:QName("m:UEquality"), "b"))),
  fn:concat("point anyURI: ",
            m:ids(idml:probe-index-point-value(xs:QName("m:UEquality"),
                                               xs:anyURI("c")))),
  fn:concat("general string: ",
            m:ids(idml:probe-index-point-general(xs:QName("m:UEquality"),
                                                 ("a", "c")))),
  fn:concat("range string: ",
            m:ids(idml:probe-index-range-value(xs:QName("m:URange"), "a", "b",
                                               fn:true(), fn:true(),
                                               fn:true(), fn:true()))),
  fn:concat("unindexed: ",
            m:ids(for $e in m:elements(xs:QName("m:u"))
                  where fn:data($e/@v) eq "b"
                  return $e))),
  "&#10;")
