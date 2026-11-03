(: A call that depends on the loop variable: it stays in the loop before and now. :)
declare function local:work($x) { $x + 1 };
declare variable $fs := (local:work#1, local:work#1);
declare variable $f := $fs[1];
sum(for $i in 1 to 1000000 return $f($i))
