(: Control: neither domain can raise, so the FLWOR is the empty sequence without evaluating it. :)
declare variable $ext external := 1;

for $a in $ext
for $b in ()
return $a
