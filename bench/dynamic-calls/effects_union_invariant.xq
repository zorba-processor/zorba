(: A query with a side effect: a loop-invariant static call of a function with a union in its body, beside a call through a function item that the compiler follows. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:work($d, $n) { sum(for $j in 1 to $n return $j * count($d | $d)) };
declare function local:inc($x) { $x + 1 };
declare variable $d := <a/>;
declare variable $k as xs:integer external := 1000;
$g := 1;
sum(for $i in 1 to 20000 return local:inc#1($i) + local:work($d, $k))
