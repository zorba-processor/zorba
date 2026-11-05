(: As pure_hoistable, with fn:fold-left in the query: still no side effect and nothing that constructs, so the call is hoisted. :)
declare function local:work($n) { sum(for $j in 1 to $n return $j * 2) };
declare variable $fs := (local:work#1, local:work#1);
declare variable $f := $fs[1];
sum(for $i in 1 to fold-left(1 to 20000, 0, function($a, $b) { max(($a, $b)) }) return $f(1000))
