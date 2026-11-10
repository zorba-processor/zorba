(: The else branch of the return clause raises for the tuples whose condition is false. :)
declare variable $zero external := 0;

for $x in (1, 2)
return if ($x eq 3) then $x else (if (6 idiv $zero eq $x) then () else ())
