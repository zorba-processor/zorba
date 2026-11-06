(: dml:delete in the modify clause removes a node from a collection. :)
import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";
ddl:create(xs:QName("local:c"));
dml:insert-last(xs:QName("local:c"), <a/>);
copy $c := <x/>
modify dml:delete(dml:collection(xs:QName("local:c"))[1])
return $c
