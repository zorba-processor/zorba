(: The same with no constructor. :)
declare function local:id($x) { $x };
declare variable $fs := (local:id#1, local:id#1);
(for $i in 1 to 100000000 return $fs[1]($i))[1]
