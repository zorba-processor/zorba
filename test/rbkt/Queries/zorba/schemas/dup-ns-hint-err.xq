(: dup-ns-x.xsd imports the namespace m from dup-ns-m1.xsd. The import of m
   that follows names dup-ns-m2.xsd; m is already loaded, so that location
   hint is disregarded and m:only-in-m2 is not declared. :)
import schema namespace x = "http://zorba.io/test/dup-ns/x" at "dup-ns-x.xsd";
import schema namespace m = "http://zorba.io/test/dup-ns/m" at "dup-ns-m2.xsd";

validate { <m:only-in-m2>a</m:only-in-m2> }
