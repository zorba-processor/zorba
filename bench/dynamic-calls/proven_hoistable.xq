(: A function behind a prolog variable the compiler can follow: still hoisted. :)
declare function local:work($n) { sum(for $j in 1 to $n return $j * 2) };
declare variable $p := local:work#1;
sum(for $i in 1 to 20000 return $p(1000))
