(: A where condition that may raise becomes an if around the FLWOR when the domain has at least one item. :)
declare variable $s as xs:integer+ external := (1, 2);
declare variable $one as xs:integer external := 1;

for $x in $s
where 6 idiv $one eq 6
return $x + 1
