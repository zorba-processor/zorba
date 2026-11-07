(: The same for fn:for-each with a named function reference and a constant input. :)
declare function local:slow($n) { sum(for $j in 1 to $n return $j * 2) };
sum(for $i in 1 to 100000 return for-each(1000000, local:slow#1))
