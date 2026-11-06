(: A query with a side effect: a function whose body has a union is deterministic and free of side effects, and its loop-invariant call is hoisted. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:id($x) { $x };
declare function local:slow($d, $n) { sum(for $j in 1 to $n return $j * count($d | $d)) };
declare variable $d := <a/>;
declare variable $k external := 1000000;
$g := 1;
sum(for $i in 1 to 100000 return local:id#1($i) + local:slow($d, $k))
