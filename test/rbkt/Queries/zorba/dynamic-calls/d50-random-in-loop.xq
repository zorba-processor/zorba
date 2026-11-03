(: A nondeterministic function called through a function item runs per iteration: p = 2^-127 to see one value by chance. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
import module namespace r = "http://zorba.io/modules/random";
declare %an:nondeterministic function local:draw() { r:random() };
declare variable $r := local:draw#0;
count(distinct-values(for $i in 1 to 128 return $r() mod 2)) eq 2
