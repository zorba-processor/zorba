(: ddl:create in the modify clause targets a collection, not a node created by
   the copy clause. :)
import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
copy $c := <x/>
modify ddl:create(xs:QName("local:c"))
return $c
