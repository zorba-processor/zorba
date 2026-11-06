(: The same for a lookup in a JSON object. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:id($x) { $x };
declare function local:slow($o as object(), $n) { sum(for $j in 1 to $n return $j * $o("a")) };
declare variable $o := { "a" : 1 };
declare variable $k external := 1000000;
$g := 1;
sum(for $i in 1 to 100000 return local:id#1($i) + local:slow($o, $k))
