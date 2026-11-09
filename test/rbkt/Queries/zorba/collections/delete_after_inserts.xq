(: ddl:delete raises ZDDY0015 only if a variable references a node of the
   collection, whatever the number of nodes and whether an index on the
   collection was used. :)
import module namespace m = "http://www.example.com/delete-after-inserts" at
                             "delete_after_inserts.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";
import module namespace iddl =
  "http://zorba.io/modules/store/static/indexes/ddl";

(: Outside a function: statements of the main module. :)
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
iddl:create(xs:QName("m:value"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
iddl:delete(xs:QName("m:value"));
variable $mainValueOne := m:delete("main, value index, one node");

ddl:create(xs:QName("m:c"));
iddl:create(xs:QName("m:value"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
dml:insert(xs:QName("m:c"), <e v="2"/>);
iddl:delete(xs:QName("m:value"));
variable $mainValueTwo := m:delete("main, value index, two separate inserts");

ddl:create(xs:QName("m:c"));
iddl:create(xs:QName("m:general"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
iddl:delete(xs:QName("m:general"));
variable $mainGeneralOne := m:delete("main, general index, one node");

ddl:create(xs:QName("m:c"));
iddl:create(xs:QName("m:general"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
dml:insert(xs:QName("m:c"), <e v="2"/>);
iddl:delete(xs:QName("m:general"));
variable $mainGeneralTwo :=
  m:delete("main, general index, two separate inserts");

variable $referenced := m:referenced("function, a variable references a node");

fn:string-join((
  $mainNone,
  $mainOne,
  $mainTwoInOne,
  $mainTwoSeparate,
  $mainValueOne,
  $mainValueTwo,
  $mainGeneralOne,
  $mainGeneralTwo,
  m:none("function, no node", ()),
  m:one("function, one node", ()),
  m:two-in-one("function, two nodes in one insert",
               ()),
  m:two-separate("function, two separate inserts",
                 ()),
  m:none("function, value index, no node", xs:QName("m:value")),
  m:one("function, value index, one node", xs:QName("m:value")),
  m:two-in-one("function, value index, two nodes in one insert",
               xs:QName("m:value")),
  m:two-separate("function, value index, two separate inserts",
                 xs:QName("m:value")),
  m:none("function, general index, no node", xs:QName("m:general")),
  m:one("function, general index, one node", xs:QName("m:general")),
  m:two-in-one("function, general index, two nodes in one insert",
               xs:QName("m:general")),
  m:two-separate("function, general index, two separate inserts",
                 xs:QName("m:general")),
  $referenced),
  "&#10;")
