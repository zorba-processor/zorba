(: A for clause over a domain of one item, before a return clause that raises before it reads the variable: the return clause is evaluated and raises. :)
declare variable $opt as xs:integer? external := 5;
declare variable $zero as xs:integer external := 0;

for $a in $opt
return (6 idiv $zero) + $a
