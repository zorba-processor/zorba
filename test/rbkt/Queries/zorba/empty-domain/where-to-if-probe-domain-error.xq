(: Probe: a false where condition after a raising domain. :)
declare variable $flag as xs:boolean external := false();
declare variable $zero as xs:integer external := 0;

for $x in (1, 6 idiv $zero)
where $flag
return $x
