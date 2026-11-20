import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:x"), (<e id="1" v="a"/>, <e id="2" t="i" v="5"/>),
         xs:QName("m:XEquality"));

(: A value comparison cannot compare the integer search key with the untyped
   key, which it compares as a string, so the probe raises the error that
   5 eq fn:data($e/@v) raises for that node. :)
idml:probe-index-point-value(xs:QName("m:XEquality"), 5)
