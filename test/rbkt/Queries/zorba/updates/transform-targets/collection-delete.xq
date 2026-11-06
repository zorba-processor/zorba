(: ddl:delete in the modify clause targets a collection. :)
import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
ddl:create(xs:QName("local:c"));
copy $c := <x/>
modify ddl:delete(xs:QName("local:c"))
return $c
