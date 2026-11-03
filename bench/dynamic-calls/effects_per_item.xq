(: A call that depends on the loop variable, in a query with a side effect. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:work($x) { $x + 1 };
declare variable $fs := (local:work#1, local:work#1);
declare variable $f := $fs[1];
$g := 1;
sum(for $i in 1 to 1000000 return $f($i))
