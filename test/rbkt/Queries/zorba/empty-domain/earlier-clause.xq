(: A for clause with an empty domain after a for clause whose domain raises: the earlier domain is evaluated. :)
declare variable $start external := 1;

(: 6 idiv $k for $k from $n down to 0. Recursive, so it is not inlined. :)
declare function local:quotients($n as xs:integer) as xs:integer*
{
  if ($n lt 0)
  then ()
  else (6 idiv $n, local:quotients($n - 1))
};

for $a in local:quotients($start)
for $b in ()
return $a
