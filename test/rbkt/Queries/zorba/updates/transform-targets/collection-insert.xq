(: dml:insert-last in the modify clause targets a collection. :)
import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";
ddl:create(xs:QName("local:c"));
copy $c := <x/>
modify dml:insert-last(xs:QName("local:c"), <a/>)
return $c
