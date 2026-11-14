import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:k"),
         (<e id="1"><k>1</k><k>2</k></e>,
          <e id="2"><k>3</k></e>,
          <e id="3"/>,
          <e id="4"><k>b</k><k>a</k><k>4</k></e>),
         xs:QName("m:KRange"));

(: Without bounds a value range probe selects every domain node, once, also
   those with several keys, which no value comparison is made with. :)
let $nodes := idml:probe-index-range-value(xs:QName("m:KRange"), (), (),
                                           fn:false(), fn:false(),
                                           fn:false(), fn:false())
return fn:string-join((
  fn:concat("ids: ", m:ids($nodes)),
  fn:concat("count: ", fn:count($nodes)),
  fn:concat("domain: ", m:ids(m:elements(xs:QName("m:k"))))),
  "&#10;")
