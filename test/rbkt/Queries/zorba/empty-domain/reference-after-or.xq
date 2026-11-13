(: A one-item for domain that raises, referenced only in the second operand of an or: the domain is evaluated for every tuple, also where the first operand is true. :)
declare variable $zero as xs:integer external := 0;

for $x in (1, $zero)
for $a in 6 idiv $x
return $x eq 0 or $a eq 6
