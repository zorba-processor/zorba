(: A query with a side effect: a pure function the compiler cannot follow, with a constant argument, runs per iteration. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare function local:work($n) { sum(for $j in 1 to $n return $j * 2) };
declare variable $fs := (local:work#1, local:work#1);
declare variable $f := $fs[1];
$g := 1;
sum(for $i in 1 to 20000 return $f(1000))
