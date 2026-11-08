(: icdml:check-integrity-constraint runs the body of the constraint, which inserts a node into a collection: the call runs once per tuple, and the count of the collection beside it reads it per tuple. :)
import module namespace e = "http://zorba.io/tests/dynamic-calls-ic" at "v12-check-integrity-constraint-in-loop.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace icdml = "http://zorba.io/modules/store/static/integrity-constraints/dml";
ddl:create(xs:QName("e:foo"));
ddl:create(xs:QName("e:log"));
for $i in 1 to 3
return (icdml:check-integrity-constraint(xs:QName("e:ic")), count(dml:collection(xs:QName("e:log"))))
