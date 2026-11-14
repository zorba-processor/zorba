(: A one-item for domain that raises, referenced only by a let clause whose variable is not referenced: the domain is evaluated for every tuple. :)
declare variable $zero as xs:integer external := 0;

for $x in (1, $zero)
for $a in 6 idiv $x
let $b := $a
return $x
