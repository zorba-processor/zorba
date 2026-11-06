(: The same with a lookup in a JSON object instead of the union. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:work($o as object(), $n) { sum(for $j in 1 to $n return $j * $o("a")) };
declare function local:inc($x) { $x + 1 };
declare variable $o := { "a" : 1 };
declare variable $k as xs:integer external := 1000;
$g := 1;
sum(for $i in 1 to 20000 return local:inc#1($i) + local:work($o, $k))
