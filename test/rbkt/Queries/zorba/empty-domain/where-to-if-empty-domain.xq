(: A where clause that does not use the for variable, over an empty domain: no tuple reaches it, so its condition is not evaluated. :)
declare variable $s as xs:integer* external := ();
declare variable $zero as xs:integer external := 0;

for $x in $s
where 6 idiv $zero eq 1
return $x
