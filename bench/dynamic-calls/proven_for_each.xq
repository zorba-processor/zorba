(: fn:for-each with a named function reference and a constant input: still hoisted. :)
declare function local:work($n) { sum(for $j in 1 to $n return $j * 2) };
sum(for $i in 1 to 20000 return for-each(1000, local:work#1))
