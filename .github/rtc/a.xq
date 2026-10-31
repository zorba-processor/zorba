declare function local:add($a as xs:double*, $b as xs:double*) as xs:double*
{ for $x at $p in $a return $x + $b[$p] };
declare function local:scale($a as xs:double*, $s as xs:double) as xs:double*
{ for $x in $a return $x * $s };
declare function local:rec($v as xs:double*, $n as xs:integer, $e as element()) as xs:double*
{
  if ($n le 0) then $v
  else local:add($v, local:rec(local:add($v, local:scale($v, 0.5)), $n - 1, $e))
};
sum(for $i in 1 to 200 return local:rec(($i, 1, 2), DEPTH, <e/>))
