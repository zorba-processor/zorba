(: dup-ns-x.xsd imports the namespace m from dup-ns-m1.xsd. The import of m
   that follows names another schema document; m is already loaded, so its
   location hint is disregarded and h refers to the components of m from
   dup-ns-m1.xsd. :)
import schema namespace x = "http://zorba.io/test/dup-ns/x" at "dup-ns-x.xsd";
import schema namespace m = "http://zorba.io/test/dup-ns/m" at "dup-ns-m2.xsd";
import schema namespace h = "http://zorba.io/test/dup-ns/h" at "dup-ns-h.xsd";

validate
{
  <h:root><h:member>a</h:member><m:head>b</m:head></h:root>
}
