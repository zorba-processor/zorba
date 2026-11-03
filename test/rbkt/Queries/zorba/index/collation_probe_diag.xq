import module namespace c = "http://www.test.com/collation-explicit" at "collation_explicit.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace index_ddl = "http://zorba.io/modules/store/static/indexes/ddl";
import module namespace index_dml = "http://zorba.io/modules/store/static/indexes/dml";

ddl:create(xs:QName("c:coll"));
dml:insert(xs:QName("c:coll"), (<a k="a"/>, <a k="b"/>));
index_ddl:create(xs:QName("c:hash"));
index_ddl:create(xs:QName("c:tree"));
index_ddl:create(xs:QName("c:general-hash"));
index_ddl:create(xs:QName("c:general-tree"));

(
  count(index_dml:probe-index-point-value(xs:QName("c:hash"), "A")),
  count(index_dml:probe-index-point-value(xs:QName("c:tree"), "B")),
  count(index_dml:probe-index-point-general(xs:QName("c:general-hash"), "A")),
  count(index_dml:probe-index-point-general(xs:QName("c:general-tree"), "B"))
)
