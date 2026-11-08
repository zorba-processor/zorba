(: ref:has-reference after ref:reference is true in every tuple. :)
import module namespace ref = "http://zorba.io/modules/reference";
declare variable $n := <a/>;
for $i in 1 to 2 return (string-length(ref:reference($n)) gt 0, ref:has-reference($n))
