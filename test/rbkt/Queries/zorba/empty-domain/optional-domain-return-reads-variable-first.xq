(: A for clause over an empty domain of at most one item, before a return clause that reads the variable first: the return clause gives the empty sequence without evaluating its other operand, so the for clause can be folded. :)
declare variable $opt as xs:integer? external := ();
declare variable $zero as xs:integer external := 0;

for $a in $opt
return $a + (6 idiv $zero)
