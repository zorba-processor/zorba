(: The domain of an inner for with a direct call gives its binding sequence per outer tuple, and each iteration runs complete before the next. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
for $i in 1 to 2 for $j in (local:bump(), local:bump()) let $c := local:bump() return ($j, $c)
