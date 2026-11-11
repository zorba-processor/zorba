(: A for clause over an empty domain of at most one item, before a where clause that does not use its variable and raises: there are no tuples, so the where clause is not evaluated. :)
declare variable $opt as xs:integer? external := ();
declare variable $zero as xs:integer external := 0;

for $a in $opt
where 6 idiv $zero eq 1
return $a
