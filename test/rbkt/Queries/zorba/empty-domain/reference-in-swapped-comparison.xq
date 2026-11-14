(: A one-item for domain that raises, referenced only as the left operand of a value comparison with an empty decimal: the optimizer swaps the operands of an integer and a decimal, the empty operand is read first, and the domain is evaluated for every tuple. :)
declare variable $zero as xs:integer external := 0;
declare variable $d as xs:decimal? external := ();

for $x in 1 idiv $zero
return $x eq $d
