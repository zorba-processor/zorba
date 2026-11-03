(: The domain of a for with the call gives its whole binding sequence first. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:inc($x) { $x + 1 };
declare variable $fs := (local:inc#1, local:inc#1);
declare variable $f := $fs[1];
$g := 1;
sum(for $x in (for $i in 1 to 3000000 return $f($i)) return $x)
