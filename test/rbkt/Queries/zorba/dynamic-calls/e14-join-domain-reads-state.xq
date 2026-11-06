(: The domain of the inner variable is read per outer tuple: it is not one index built before the loop. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
(for $a in 1 to 3 for $b in (for $j in 1 to 3 return $j + 2 * xs:integer(local:g())) where $a eq $b return ($f(), $a, $b), "end")
