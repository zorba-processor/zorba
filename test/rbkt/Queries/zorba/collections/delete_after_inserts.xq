import module namespace m = "http://www.example.com/delete-after-inserts" at
                             "delete_after_inserts.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";
import module namespace iddl =
  "http://zorba.io/modules/store/static/indexes/ddl";

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

ddl:create(xs:QName("m:c"));
iddl:create(xs:QName("m:i"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
iddl:delete(xs:QName("m:i"));
variable $mainIndexOne := m:delete("main, index, one node");

ddl:create(xs:QName("m:c"));
iddl:create(xs:QName("m:i"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
dml:insert(xs:QName("m:c"), <e v="2"/>);
iddl:delete(xs:QName("m:i"));
variable $mainIndexTwoSeparate := m:delete("main, index, two separate inserts");

variable $results := (
  $mainNone,
  $mainOne,
  $mainTwoInOne,
  $mainTwoSeparate,
  $mainIndexOne,
  $mainIndexTwoSeparate,
  m:none("function, no node", ()),
  m:one("function, one node", ()),
  m:two-in-one("function, two nodes in one insert", ()),
  m:two-separate("function, two separate inserts", ()),
  m:none("function, index, no node", xs:QName("m:i")),
  m:one("function, index, one node", xs:QName("m:i")),
  m:two-in-one("function, index, two nodes in one insert", xs:QName("m:i")),
  m:two-separate("function, index, two separate inserts", xs:QName("m:i")),
  m:one-param("function, one parameter node", <e v="1"/>),
  m:two-params("function, two parameter nodes", <e v="1"/>, <e v="2"/>),
  m:two-try("function, second insert in a try block"));

fn:string-join($results, "&#10;")
