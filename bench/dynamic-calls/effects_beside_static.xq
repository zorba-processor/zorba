(: A static call beside such a call stays in the loop. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:work($n) { sum(for $j in 1 to $n return $j * 2) };
declare function local:inc($x) { $x + 1 };
declare variable $fs := (local:inc#1, local:inc#1);
declare variable $f := $fs[1];
declare variable $k as xs:integer external := 1000;
$g := 1;
sum(for $i in 1 to 20000 return $f($i) + local:work($k))
