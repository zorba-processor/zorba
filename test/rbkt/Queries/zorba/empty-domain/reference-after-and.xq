(: A one-item for domain that raises, referenced only in the second operand of an and: the domain is evaluated for every tuple, also where the first operand is false. :)
declare variable $zero as xs:integer external := 0;

for $x in (1, $zero)
for $a in 6 idiv $x
return $x ne 0 and $a eq 6
