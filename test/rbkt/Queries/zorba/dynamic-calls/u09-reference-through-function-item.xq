(: ref:reference called through a function item gives a node a reference: ref:has-reference next to the call reads it per tuple. :)
import module namespace ref = "http://zorba.io/modules/reference";
declare variable $n := <a/>;
declare variable $f := ref:reference#1;
for $i in 1 to 2 return (ref:has-reference($n), exists($f($n)))
