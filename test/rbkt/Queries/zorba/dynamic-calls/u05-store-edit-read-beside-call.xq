(: dml:edit through a function item changes a node of a collection and keeps its identity: a count of its children next to the call reads them per tuple. :)
import module namespace e = "http://zorba.io/tests/dynamic-calls-index" at "e16-index-probe.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace iddl = "http://zorba.io/modules/store/static/indexes/ddl";
declare namespace an = "http://zorba.io/annotations";
declare %an:sequential function local:grow($k) { dml:edit(dml:collection(xs:QName("e:foo"))[1], <a id="1">{for $j in 1 to $k return <b/>}</a>); };
declare variable $f := local:grow#1;
ddl:create(xs:QName("e:foo"));
iddl:create(xs:QName("e:by-id"));
dml:insert(xs:QName("e:foo"), <a id="1"/>);
variable $n := dml:collection(xs:QName("e:foo"))[1];
for $i in 1 to 3 return ($f($i), count($n/b))
