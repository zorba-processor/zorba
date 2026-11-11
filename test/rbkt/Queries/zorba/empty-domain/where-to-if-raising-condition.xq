(: A where clause that does not use the for variable, over a non-empty domain: its condition is evaluated and raises. :)
declare variable $s as xs:integer* external := (1, 2);
declare variable $zero as xs:integer external := 0;

for $x in $s
where 6 idiv $zero eq 1
return $x
