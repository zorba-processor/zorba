(: ref:has-reference reads the reference that ref:reference gives the node; it is evaluated per tuple. :)
import module namespace ref = "http://zorba.io/modules/reference";
declare variable $n := <a/>;
for $i in 1 to 2 return (ref:has-reference($n), exists(ref:reference($n)))
