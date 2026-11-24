(: Control: fn:count, fn:empty and fn:exists over a for variable are folded by the type of the variable. :)
declare variable $n as xs:integer external := 2;

for $x in 1 to $n
return (count($x), empty($x), exists($x))
