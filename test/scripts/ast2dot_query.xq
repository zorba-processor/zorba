declare variable $x := "say \"hi\"";
for $i in 1 to 3
return fn:concat($x, $i + 1)
