(: A pure function the compiler cannot see, with a constant argument: hoisted before, called per iteration now. :)
declare function local:work($n) { sum(for $j in 1 to $n return $j * 2) };
declare variable $fs := (local:work#1, local:work#1);
declare variable $f := $fs[1];
sum(for $i in 1 to 20000 return $f(1000))
