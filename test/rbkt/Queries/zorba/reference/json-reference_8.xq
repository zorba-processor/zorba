import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";
import module namespace ref = "http://zorba.io/modules/reference";

(: The reference of a JSON collection root that dml:delete-last deleted does
   not resolve. :)

ddl:create(xs:QName("local:c"));
dml:insert-last(xs:QName("local:c"), ({ "a" : 1 }, { "a" : 2 }));
variable $r := ref:reference(dml:collection(xs:QName("local:c"))[2]);
dml:delete-last(xs:QName("local:c"));
dml:insert-last(xs:QName("local:c"), { "b" : [ 1, 2, 3 ] });
[ ref:dereference($r) ]
