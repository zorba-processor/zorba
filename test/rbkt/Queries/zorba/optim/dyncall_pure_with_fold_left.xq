(: A query with no side effect that uses fn:fold-left with a function the compiler sees: a call it cannot follow is an ordinary pure expression, hoisted out of the loop. Without it this takes hours. :)
declare function local:slow($n) { sum(for $j in 1 to $n return $j * 2) };
declare variable $fs := (local:slow#1, local:slow#1);
declare variable $f := $fs[1];
sum(for $i in 1 to fold-left(1 to 100000, 0, function($a, $b) { max(($a, $b)) }) return $f(1000000))
