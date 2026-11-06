(: A query with a side effect: a static call beside a call the compiler cannot follow is hoisted out of the loop; the call stays. $k is external with no type, but it does not reach a call. Without hoisting this takes hours, and the test has a time limit. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:slow($n) { sum(for $j in 1 to $n return $j * 2) };
declare function local:inc($x) { $x + 1 };
declare variable $fs := (local:inc#1, local:inc#1);
declare variable $f := $fs[1];
declare variable $k external := 1000000;
$g := 1;
sum(for $i in 1 to 100000 return $f($i) + local:slow($k))
