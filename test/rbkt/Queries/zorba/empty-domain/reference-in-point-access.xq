(: A one-item for domain that raises, referenced only as the sequence of a positional predicate whose position is 0: the predicate does not read the sequence, and the domain is evaluated for every tuple. :)
declare variable $zero as xs:integer external := 0;
declare variable $p as xs:integer external := 0;

for $x in 1 idiv $zero
return $x[$p]
