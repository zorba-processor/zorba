(: A one-item for domain that raises, referenced only in the return clause of a nested FLWOR over a domain that is empty for some tuples: the domain is evaluated for every tuple. :)
declare variable $zero as xs:integer external := 0;

for $x in (1, $zero)
for $a in 6 idiv $x
return for $y in $x[. ne 0] return $a
