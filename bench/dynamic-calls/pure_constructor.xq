(: A constructor in a query with no side effect: the calls are not reused, and the tuples are not materialized. :)
declare function local:id($x) { $x };
declare variable $fs := (local:id#1, local:id#1);
count(for $i in 1 to 3000000 let $n := <n/> return $fs[1]($n))
