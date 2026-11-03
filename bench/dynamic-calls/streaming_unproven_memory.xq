(: A loop with such a call over a large input whose clauses read no state: not materialized, so memory stays bounded. :)
declare function local:inc($x) { $x + 1 };
declare variable $fs := (local:inc#1, local:inc#1);
declare variable $f := $fs[1];
sum(for $i in 1 to 3000000 return $f($i))
