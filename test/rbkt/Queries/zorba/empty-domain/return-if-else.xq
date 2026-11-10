(: The else branch of the return clause raises for the tuples whose condition is false. :)
declare variable $s external := "a";

for $x in (1, 2)
return if ($x eq 3) then $x else (if (xs:integer($s) eq 1) then () else ())
