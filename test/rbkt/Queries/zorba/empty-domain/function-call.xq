(: A domain of the empty sequence type that calls a function whose division by zero raises. :)
declare variable $zero external := 0;

(: Recursive, so it is not inlined. :)
declare function local:check($k as xs:integer) as empty-sequence()
{
  if ($k gt 0)
  then local:check($k - 1)
  else if (1 idiv $zero eq 0) then () else ()
};

for $x in local:check(1)
return 1
