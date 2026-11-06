(: iddl:create in the modify clause creates an index. :)
import module namespace t = "http://zorba.io/tests/transform-targets" at "targets.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace iddl = "http://zorba.io/modules/store/static/indexes/ddl";
ddl:create(xs:QName("t:c"));
copy $c := <x/>
modify iddl:create(xs:QName("t:idx"))
return $c
