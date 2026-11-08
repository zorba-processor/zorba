(: idml:refresh-index in the modify clause refreshes an index. :)
import module namespace t = "http://zorba.io/tests/transform-targets" at "targets.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace iddl = "http://zorba.io/modules/store/static/indexes/ddl";
import module namespace idml = "http://zorba.io/modules/store/static/indexes/dml";
ddl:create(xs:QName("t:c"));
iddl:create(xs:QName("t:idx"));
copy $c := <x/>
modify idml:refresh-index(xs:QName("t:idx"))
return $c
