(: Control: no dynamic call. :)
declare function local:work($n) { sum(for $j in 1 to $n return $j * 2) };
sum(for $i in 1 to 20000 return local:work(1000))
