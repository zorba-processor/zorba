(: A one-item for domain that raises, whose variable is not referenced: the domain is evaluated. :)
declare variable $zero as xs:integer external := 0;

for $a in (1, $zero) ! (6 idiv .)
return 1
