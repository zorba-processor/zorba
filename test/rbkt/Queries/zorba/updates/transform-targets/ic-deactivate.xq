(: icddl:deactivate in the modify clause deactivates an integrity constraint. :)
import module namespace t = "http://zorba.io/tests/transform-targets" at "targets.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace icddl = "http://zorba.io/modules/store/static/integrity-constraints/ddl";
ddl:create(xs:QName("t:c"));
icddl:activate(xs:QName("t:ic"));
copy $c := <x/>
modify icddl:deactivate(xs:QName("t:ic"))
return $c
