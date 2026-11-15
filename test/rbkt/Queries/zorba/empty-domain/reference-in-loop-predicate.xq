(: A one-item for domain that raises, referenced only in a predicate over a sequence of more than one item: the predicate is a loop, so the domain is not folded into it and is evaluated for every tuple. :)
declare variable $zero as xs:integer external := 0;

for $x in (1, $zero)
for $a in 6 idiv $x
return (1 to $x)[. eq $a]
