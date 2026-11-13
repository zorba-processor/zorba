import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";

import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

import module namespace iddl =
  "http://zorba.io/modules/store/static/indexes/ddl";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

(: Truncating the collection empties the index of the node with several
   untyped keys, so value probes of the integer keys inserted afterwards
   compare with those keys only. :)
ddl:create(xs:QName("m:ta"));
iddl:create(xs:QName("m:TaRange"));
dml:insert(xs:QName("m:ta"), <e id="1"><k>1</k><k>a</k></e>);
dml:truncate(xs:QName("m:ta"));
dml:insert(xs:QName("m:ta"), (<e id="2" t="i" v="3"/>, <e id="3" t="i" v="5"/>));

fn:string-join((
  fn:concat("range: ",
            m:ids(idml:probe-index-range-value(xs:QName("m:TaRange"), 1, 4,
                                               fn:true(), fn:true(),
                                               fn:true(), fn:true()))),
  fn:concat("point: ",
            m:ids(idml:probe-index-point-value(xs:QName("m:TaRange"), 5)))),
  "&#10;")
