(: A for clause over an empty domain of at most one item, before a return clause that raises before it reads the variable: there are no tuples, so the return clause is not evaluated. :)
declare variable $opt as xs:integer? external := ();
declare variable $zero as xs:integer external := 0;

for $a in $opt
return (6 idiv $zero) + $a
