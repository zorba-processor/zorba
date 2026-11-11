(: A where clause that does not use the for variable and may raise, over a domain that can be empty but is not. :)
declare variable $s as xs:integer* external := (1, 2);
declare variable $one as xs:integer external := 1;

for $x in $s
where 6 idiv $one eq 6
return $x
