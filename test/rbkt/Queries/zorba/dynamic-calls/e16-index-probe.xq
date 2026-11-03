(: A condition with the call runs per tuple: it is not one index probe. :)
import module namespace e = "http://zorba.io/tests/dynamic-calls-index" at "e16-index-probe.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace iddl = "http://zorba.io/modules/store/static/indexes/ddl";
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; string($g) };
declare variable $f := local:bump#0;
ddl:create(xs:QName("e:foo"));
iddl:create(xs:QName("e:by-id"));
dml:insert(xs:QName("e:foo"), ({ "id" : "1" }, { "id" : "2" }, { "id" : "3" }));
for $i in dml:collection(xs:QName("e:foo"))
where $i("id") eq $f()
order by $i("id")
return $i("id")
