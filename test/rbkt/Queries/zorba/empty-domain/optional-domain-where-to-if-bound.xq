(: A for clause over a domain of one item, before a where clause that does not use its variable and raises: the where clause is evaluated and raises. :)
declare variable $opt as xs:integer? external := 5;
declare variable $zero as xs:integer external := 0;

for $a in $opt
where 6 idiv $zero eq 1
return $a
