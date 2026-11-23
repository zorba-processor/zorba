(: A function reads $g too: $g is materialized. :)
declare variable $g := (1, 2, 3);

declare function local:size() { fn:count($g) };

for $x in $g
return $x * local:size()
