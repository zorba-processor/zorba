(: A for clause with an empty domain after a for clause over an external variable that is not bound: the earlier domain is evaluated. :)
declare variable $ext external;

for $a in $ext
for $b in ()
return $a
