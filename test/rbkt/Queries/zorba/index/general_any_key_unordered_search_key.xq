import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:a"), <e id="1" v="2000"/>, xs:QName("m:ARange"));

(: A range index whose key type is xs:anyAtomicType* is not probed with a key
   that has no ordering, as an index without a key type is not. :)
idml:probe-index-point-general(xs:QName("m:ARange"), xs:gYear("2000"))
