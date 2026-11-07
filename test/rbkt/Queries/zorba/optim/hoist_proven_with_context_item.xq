(: The context item is external and can hold any item, a function item that the compiler cannot see among them, so a call through a function item could have a side effect. Calls the compiler can follow are proven: a static call beside one, and a path from the context item, are hoisted. Without hoisting this takes hours. :)
declare context item external := <r><item/></r>;
declare function local:slow($n) { sum(for $j in 1 to $n return $j * 2) };
declare function local:inc($x) { $x + 1 };
declare variable $k as xs:integer external := 1000000;
sum(for $i in 1 to 100000 return local:inc#1($i) + local:slow($k) + count(//item))
