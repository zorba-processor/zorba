(: A one-item for domain that raises, referenced only in the else branch of the return clause: the domain is evaluated for every tuple, also where the then branch is taken. :)
declare variable $zero as xs:integer external := 0;

for $x in (1, $zero)
for $a in 6 idiv $x
return if ($x eq 0) then () else $a
