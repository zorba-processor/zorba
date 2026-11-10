(: A one-item for domain that raises, before a for clause with an empty domain: the domain is evaluated. :)
declare variable $zero external := 0;

for $a in (1, $zero) ! (6 idiv .)
for $b in ()
return $a
