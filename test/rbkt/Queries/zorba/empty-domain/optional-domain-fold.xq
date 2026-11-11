(: A for clause over a domain of at most one item, before clauses that cannot raise: its variable is folded into the return clause. :)
declare variable $opt as xs:integer? external := 5;

for $a in $opt
for $c in (1, 2)
return $a
