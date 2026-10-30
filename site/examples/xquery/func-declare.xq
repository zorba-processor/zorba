declare function local:discount($price as xs:decimal, $pct as xs:integer)
  as xs:decimal
{
  $price - $price * $pct div 100
};

declare function local:fib($n as xs:integer) as xs:integer
{
  if ($n lt 2) then $n else local:fib($n - 1) + local:fib($n - 2)
};

(
  local:discount(24.00, 25),
  for $i in 0 to 10 return local:fib($i)
)
