(: A sliding window with a dynamic call in a query with no side effect: the input is released. :)
declare variable $pad := string-join(for $i in 1 to 16384 return 'x', '');
declare function local:d() { for $i in 1 to 20000 return concat($pad, $i) };
declare function local:inc($x) { $x + 1 };
declare variable $fs := (local:inc#1, local:inc#1);
sum(for sliding window $w in local:d() start at $s when true only end at $e when $e - $s eq 9 return $fs[1](count($w)))
