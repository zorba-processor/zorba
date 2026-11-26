(: The count of $h is hoisted into a let clause before the for clause, which
 : still reads $g once. :)
declare variable $g := (1, 2, 3);
declare variable $h := (10, 20);

for $x in $g
return $x + fn:count($h)
