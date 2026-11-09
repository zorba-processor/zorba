import module namespace m = "http://www.example.com/delete-after-inserts" at
                             "delete_after_inserts.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

(: Outside a function: the statements of the main module. :)
ddl:create(xs:QName("m:c"));
variable $mainNone := m:delete("main, no node");

ddl:create(xs:QName("m:c"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
variable $mainOne := m:delete("main, one node");

ddl:create(xs:QName("m:c"));
dml:insert(xs:QName("m:c"), (<e v="1"/>, <e v="2"/>));
variable $mainTwoInOne := m:delete("main, two nodes in one insert");

ddl:create(xs:QName("m:c"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
dml:insert(xs:QName("m:c"), <e v="2"/>);
variable $mainTwoSeparate := m:delete("main, two separate inserts");

variable $results := (
  $mainNone,
  $mainOne,
  $mainTwoInOne,
  $mainTwoSeparate,
  m:none("function, no node", fn:false()),
  m:one("function, one node", fn:false()),
  m:two-in-one("function, two nodes in one insert", fn:false()),
  m:two-separate("function, two separate inserts", fn:false()),
  m:none("function, index, no node", fn:true()),
  m:one("function, index, one node", fn:true()),
  m:two-in-one("function, index, two nodes in one insert", fn:true()),
  m:two-separate("function, index, two separate inserts", fn:true()),
  m:one-param("function, one parameter node", <e v="1"/>),
  m:two-params("function, two parameter nodes", <e v="1"/>, <e v="2"/>),
  m:two-try("function, second insert in a try block"));

fn:string-join($results, "&#10;")
