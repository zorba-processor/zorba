(: The type of the declaration is checked at every level. :)
declare variable $g as xs:integer+ := (1, "a");

for $x in $g
return $x
