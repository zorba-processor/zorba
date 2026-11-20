(: dup-ns-x.xsd imports the namespace m from dup-ns-m1.xsd. The import of m
   that follows names a schema document that does not exist; m is already
   loaded, so that location hint is disregarded without being resolved. :)
import schema namespace x = "http://zorba.io/test/dup-ns/x" at "dup-ns-x.xsd";
import schema namespace m = "http://zorba.io/test/dup-ns/m" at "dup-ns-missing.xsd";

validate { <m:head>a</m:head> }
