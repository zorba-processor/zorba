(: XQSX 6.7: the domain of a for with a direct call gives its whole binding sequence first, then later clauses run per tuple (as h02). :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
for $x in (local:bump(), local:bump()) let $a := local:bump() return ($x, $a, $g)
