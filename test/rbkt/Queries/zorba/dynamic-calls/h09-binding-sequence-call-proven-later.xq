(: The call in the domain cannot be followed before inlining: the domain gives its binding sequence first at every level, also when inlining proves the call pure. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare function local:zero() { 0 };
declare function local:get() { local:zero#0 };
for $x in (for $j in 1 to 3 return local:get()() + $g) return ($f(), $x)
