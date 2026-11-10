(: A domain of the empty sequence type whose condition casts "a" to xs:integer raises the cast error. :)
declare variable $s external := "a";

for $x in (if (xs:integer($s) eq 1) then () else ())
return 1
