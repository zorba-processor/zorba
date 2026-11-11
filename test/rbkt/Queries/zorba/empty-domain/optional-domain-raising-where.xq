(: A for clause over an empty domain of at most one item, before a where clause that raises: there are no tuples, so the where clause is not evaluated. :)
declare variable $opt as xs:integer? external := ();
declare variable $zero as xs:integer external := 0;

for $a in $opt
for $c in (1, 2)
where 6 idiv ($c * $zero) eq 1
return $a
