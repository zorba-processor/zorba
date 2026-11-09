(: A for clause with an empty domain after a for clause whose domain raises: the earlier domain is evaluated. :)
declare variable $zero external := 0;

for $a in (1, $zero) ! (6 idiv .)
for $b in ()
return $a
