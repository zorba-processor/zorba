(: icddl:activate in the modify clause activates an integrity constraint. :)
import module namespace t = "http://zorba.io/tests/transform-targets" at "targets.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace icddl = "http://zorba.io/modules/store/static/integrity-constraints/ddl";
ddl:create(xs:QName("t:c"));
copy $c := <x/>
modify icddl:activate(xs:QName("t:ic"))
return $c
