(: A query with no side effect that uses fn:filter: a call the compiler cannot follow is an ordinary pure expression, hoisted out of the loop. Without it this takes hours. :)
declare function local:slow($n) { sum(for $j in 1 to $n return $j * 2) };
declare variable $fs := (local:slow#1, local:slow#1);
declare variable $f := $fs[1];
sum(for $i in filter(1 to 100000, function($x) { $x gt 0 }) return $f(1000000))
