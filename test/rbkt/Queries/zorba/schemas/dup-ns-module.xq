(: The library module imports the namespace m from dup-ns-m1.xsd and this
   module imports it from dup-ns-m2.xsd. Each module has its own in-scope
   schema definitions, so m:only-in-m2 is declared here. :)
import module namespace dm = "http://zorba.io/test/dup-ns/module" at "dup-ns-module.xqlib";
import schema namespace m = "http://zorba.io/test/dup-ns/m" at "dup-ns-m2.xsd";

(dm:head(), fn:data(validate { <m:only-in-m2>z</m:only-in-m2> }))
