(: A one-item for domain that raises, referenced only in a predicate over a sequence that is empty for some tuples: the domain is evaluated for every tuple. :)
declare variable $zero as xs:integer external := 0;

for $x in (1, $zero)
for $a in 6 idiv $x
return $x[. ne 0][. eq $a]
