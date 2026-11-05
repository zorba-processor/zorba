(: The same for a lookup whose input is only known to be a JSON object after inlining. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare function local:obj() { { "a" : 1 } };
for $x in (for $j in 1 to 3 return local:obj()("a") + $g) return ($f(), $x)
