(: Two references: $g is materialized. :)
declare variable $g := (1, 2, 3);

for $x in $g
return $x + fn:count($g)
