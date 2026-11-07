(: A query with a side effect: a function item hoisted out of the loop is still proven, so that the static call beside it is hoisted. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:slow($n) { sum(for $j in 1 to $n return $j * 2) };
declare variable $k external := 1000000;
$g := 1;
sum(for $i in 1 to 100000 return local:slow#1($i mod 1) + local:slow($k))
