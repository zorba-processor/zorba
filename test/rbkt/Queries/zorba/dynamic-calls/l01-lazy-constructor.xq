(: A query with no side effect stays lazy. Materializing the tuples takes minutes and the test has a time limit. :)
declare function local:id($x) { $x };
declare variable $fs := (local:id#1, local:id#1);
(for $i in 1 to 100000000 let $n := <n/> return $fs[1]($n))[1]
