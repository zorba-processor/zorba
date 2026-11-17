import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:x"), (<e id="1" v="a"/>, <e id="2" t="i" v="5"/>),
         xs:QName("m:XRange"));

(: An untyped bound compares as a string, which a value comparison cannot
   compare with the integer key, so the probe raises the error that
   $lower le fn:data($e/@v) raises for that node. :)
idml:probe-index-range-value(xs:QName("m:XRange"),
                             xs:untypedAtomic("a"), xs:untypedAtomic("z"),
                             fn:true(), fn:true(), fn:true(), fn:true())
