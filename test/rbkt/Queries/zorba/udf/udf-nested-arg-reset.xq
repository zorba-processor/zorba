(: A recursive call whose argument is computed from the caller's parameter,
   evaluated again and again from a loop. Every evaluation resets the call,
   which reset each argument subplan both directly and through the callee's
   parameter, at every level of the recursion reached so far: 2^depth work
   per iteration. The element parameter keeps the functions out of the
   automatic result cache. :)
declare function local:id($v as xs:double*, $e as element()) as xs:double*
{
  for $x in $v return $x
};

declare function local:walk($v as xs:double*, $n as xs:integer, $e as element())
  as xs:double*
{
  if ($n le 0) then $v
  else local:walk(local:id(local:id($v, $e), $e), $n - 1, $e)
};

let $e := <e/>
return sum(for $i in 1 to 200 return sum(local:walk(($i, 1, 2), 24, $e)))
