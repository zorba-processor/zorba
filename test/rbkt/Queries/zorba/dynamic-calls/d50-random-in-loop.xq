(: A nondeterministic function called through a function item runs per iteration: p = 2^-127 to see one value by chance. :)
import module namespace r = "http://zorba.io/modules/random";
declare namespace an = "http://zorba.io/annotations";
declare %an:nondeterministic function local:draw() { r:random() };
declare variable $r := local:draw#0;
count(distinct-values(for $i in 1 to 128 return $r() mod 2)) eq 2
