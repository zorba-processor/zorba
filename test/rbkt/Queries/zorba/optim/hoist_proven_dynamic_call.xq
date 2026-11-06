(: A deterministic function behind a function item in a prolog variable is hoisted: without it this takes hours, and the test has a time limit. :)
declare function local:slow($n) { sum(for $j in 1 to $n return $j * 2) };
declare variable $p := local:slow#1;
sum(for $i in 1 to 100000 return $p(1000000))
