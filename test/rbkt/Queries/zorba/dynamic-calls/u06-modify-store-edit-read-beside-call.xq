(: dml:edit in the modify clause of a transform, in a function called through a function item, changes a node of a collection: the store does not check that the target of a collection update is a copy (XUDY0014). The query has no statement (a statement is applied as an update, which already counts as a side effect): prolog variables create the collection and its node the same way. $n is a for variable, which is not folded into its use, and the count of its children next to the call reads them per tuple. :)
import module namespace e = "http://zorba.io/tests/dynamic-calls-index" at "e16-index-probe.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
declare function local:grow($k) { copy $c := <x/> modify dml:edit(dml:collection(xs:QName("e:foo"))[1], <a id="1">{for $j in 1 to $k return <b/>}</a>) return () };
declare variable $f := local:grow#1;
declare variable $created := copy $c := <x/> modify ddl:create(xs:QName("e:foo")) return ();
declare variable $inserted := copy $c := <x/> modify dml:insert(xs:QName("e:foo"), <a id="1"/>) return ();
$created, $inserted,
for $n in dml:collection(xs:QName("e:foo"))
return for $i in 1 to 3 return ($f($i), count($n/b))
