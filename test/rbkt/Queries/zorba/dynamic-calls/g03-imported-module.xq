(: The sequential function is only in an imported module, behind a variable of that module. :)
import module namespace m = "http://zorba.io/tests/dynamic-calls" at "g03-module.xqlib";
for $i in 1 to 3 return ($m:f(), $i)
