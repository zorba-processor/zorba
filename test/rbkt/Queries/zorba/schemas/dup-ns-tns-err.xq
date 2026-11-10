(: dup-ns-x.xsd imports the namespace m from dup-ns-m1.xsd. The schema
   document at the location hint of the import of a has the target namespace
   m, not a. :)
import schema namespace x = "http://zorba.io/test/dup-ns/x" at "dup-ns-x.xsd";
import schema namespace a = "http://zorba.io/test/dup-ns/a" at "dup-ns-m2.xsd";

validate
{
  <h:root xmlns:h="http://zorba.io/test/dup-ns/h"><h:member>a</h:member></h:root>
}
