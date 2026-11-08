(: iddl:delete in the modify clause deletes an index. :)
import module namespace t = "http://zorba.io/tests/transform-targets" at "targets.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace iddl = "http://zorba.io/modules/store/static/indexes/ddl";
ddl:create(xs:QName("t:c"));
iddl:create(xs:QName("t:idx"));
copy $c := <x/>
modify iddl:delete(xs:QName("t:idx"))
return $c
