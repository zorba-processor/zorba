(: The for clause of $g is evaluated once per tuple of the clause before it:
 : $g is materialized. :)
declare variable $g := (1, 2);

for $a in (10, 20)
for $x in $g
return $a + $x
