(: Control: fn:boolean over a for variable bound to an element is folded to true by the type of the variable. :)
declare variable $n as xs:integer external := 2;

for $x in (<a/>, <b/>)[position() le $n]
return boolean($x)
