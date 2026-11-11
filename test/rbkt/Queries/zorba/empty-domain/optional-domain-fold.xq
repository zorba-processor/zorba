(: A for clause over a domain of at most one item, before a where clause that cannot raise: the for clause is folded and the where clause becomes an if. :)
declare variable $opt as xs:integer? external := 5;
declare variable $flag as xs:boolean external := true();

for $a in $opt
where $flag
return $a
