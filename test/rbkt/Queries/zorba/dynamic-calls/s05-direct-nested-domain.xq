(: The tuples of a loop are computed before its return clause runs: the domain gives its whole binding sequence first (as d44). :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
for $i in (for $j in 1 to 3 return $j * 10 + local:g()) return (local:bump(), $i)
