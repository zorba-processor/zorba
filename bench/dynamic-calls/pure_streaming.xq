(: A loop over a large input in a query with no side effect: not materialized. :)
declare function local:inc($x) { $x + 1 };
declare variable $fs := (local:inc#1, local:inc#1);
declare variable $f := $fs[1];
sum(for $i in 1 to 3000000 return $f($i))
