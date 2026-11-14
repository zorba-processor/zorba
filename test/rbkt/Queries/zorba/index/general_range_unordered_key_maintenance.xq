import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

(: Nor can an automatic range index get one when a node is inserted. :)
m:create(xs:QName("m:ya"), (), xs:QName("m:YaRange"));
dml:insert(xs:QName("m:ya"), <e id="1" v="1"/>);
dml:insert(xs:QName("m:ya"), <e id="2" t="y" v="2000"/>);
