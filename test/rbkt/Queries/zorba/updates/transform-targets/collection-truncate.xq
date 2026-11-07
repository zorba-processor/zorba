(: dml:truncate in the modify clause targets a collection. :)
import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";
ddl:create(xs:QName("local:c"));
dml:insert-last(xs:QName("local:c"), <a/>);
copy $c := <x/>
modify dml:truncate(xs:QName("local:c"))
return $c
