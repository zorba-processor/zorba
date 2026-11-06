(: ref:assign-reference through a function item gives a node a reference: ref:has-reference next to the call reads it per tuple. :)
import module namespace e = "http://zorba.io/tests/dynamic-calls-index" at "e16-index-probe.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace iddl = "http://zorba.io/modules/store/static/indexes/ddl";
import module namespace ref = "http://zorba.io/modules/reference";
declare namespace an = "http://zorba.io/annotations";
declare %an:sequential function local:set($n) { ref:assign-reference($n, xs:anyURI(concat("urn:uuid:e35e9021-0a1c-48c7-8902-83310500000", string($n/@id)))) };
declare variable $f := local:set#1;
ddl:create(xs:QName("e:foo"));
iddl:create(xs:QName("e:by-id"));
dml:insert(xs:QName("e:foo"), (<a id="1"/>, <a id="2"/>, <a id="3"/>));
variable $ns as element(a)* := dml:collection(xs:QName("e:foo"));
for $i in 1 to 3 return ($f($ns[$i]), count($ns[ref:has-reference(.)]))
