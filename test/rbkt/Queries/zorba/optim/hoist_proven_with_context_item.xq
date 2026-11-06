(: The context item is external and can hold any item, but it does not reach a call: a static call beside a call the compiler cannot follow, and a path from the context item, are hoisted. Without hoisting this takes hours. :)
declare context item := <r><item/></r>;
declare function local:slow($n) { sum(for $j in 1 to $n return $j * 2) };
declare function local:inc($x) { $x + 1 };
declare variable $fs := (local:inc#1, local:inc#1);
declare variable $f := $fs[1];
declare variable $k as xs:integer external := 1000000;
sum(for $i in 1 to 100000 return $f($i) + local:slow($k) + count(//item))
