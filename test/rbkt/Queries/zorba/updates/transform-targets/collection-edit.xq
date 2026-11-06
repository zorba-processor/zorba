(: dml:edit in the modify clause changes a node of a collection. :)
import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";
ddl:create(xs:QName("local:c"));
dml:insert-last(xs:QName("local:c"), <a/>);
copy $c := <x/>
modify dml:edit(dml:collection(xs:QName("local:c"))[1], <b/>)
return $c
